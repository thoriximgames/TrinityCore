#include "Session.h"
#include "SessionManager.h"
#include "../Engine/EntityManager.h"
#include "../Engine/DatabaseManager.h"
#include "../Shared/Cryptography/Crypto.h"
#include "../Shared/Architecture/PacketDispatcher.h"
#include <iostream>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <atomic>

namespace MMO::Host {

Session::Session(tcp::socket socket)
    : _state(SessionState::Authenticating), _authenticated(false), 
      _player(nullptr), _tempAccountId(0), _socket(std::move(socket)), _isWriting(false) 
{
}

Session::~Session() {
    if (_player) {
        Engine::EntityManager::Instance().RemoveEntity(_player->GetId());
        std::cout << "[Session] Player " << _player->GetName() << " (ID: " << _player->GetId() << ") left the world." << std::endl;
    }
    std::cout << "[Session] Client disconnected." << std::endl;
}

asio::awaitable<void> Session::Process() {
    try {
        while (true) {
            uint32_t size = 0;
            co_await asio::async_read(_socket, asio::buffer(&size, sizeof(size)), asio::use_awaitable);

            if (size == 0 || size > 1024 * 1024) break;

            std::vector<uint8_t> buffer(size);
            co_await asio::async_read(_socket, asio::buffer(buffer.data(), size), asio::use_awaitable);

            auto masterPacket = Protocol::GetMasterPacket(buffer.data());
            
            // Create TraceContext for the request flow
            auto ctx = Shared::Architecture::TraceContext::Create(_tempAccountId, "NetworkHost");
            
            co_await Shared::Architecture::PacketDispatcher::Instance().Dispatch(ctx, masterPacket, shared_from_this());
        }
    } catch (...) {}
}

asio::awaitable<void> Session::HandlePacket(const Protocol::MasterPacket*) {
    // Handled by PacketDispatcher
    co_return;
}

asio::awaitable<void> Session::Send(flatbuffers::FlatBufferBuilder& fbb) {
    uint32_t size = fbb.GetSize();
    co_await asio::async_write(_socket, asio::buffer(&size, sizeof(size)), asio::use_awaitable);
    co_await asio::async_write(_socket, asio::buffer(fbb.GetBufferPointer(), size), asio::use_awaitable);
}

void Session::QueuePacket(std::shared_ptr<std::vector<uint8_t>> packetData) {
    std::lock_guard lock(_writeMutex);
    _writeQueue.push_back(packetData);
    if (!_isWriting) {
        _isWriting = true;
        auto executor = _socket.get_executor();
        asio::co_spawn(executor, [self = shared_from_this()]() { return self->WriteLoop(); }, asio::detached);
    }
}

asio::awaitable<void> Session::WriteLoop() {
    try {
        while (true) {
            std::shared_ptr<std::vector<uint8_t>> packet;
            {
                std::lock_guard lock(_writeMutex);
                if (_writeQueue.empty()) { _isWriting = false; co_return; }
                packet = _writeQueue.front();
                _writeQueue.pop_front();
            }
            co_await asio::async_write(_socket, asio::buffer(packet->data(), packet->size()), asio::use_awaitable);
        }
    } catch (...) {
        std::lock_guard lock(_writeMutex);
        _isWriting = false;
        _writeQueue.clear();
    }
}

} // namespace MMO::Host