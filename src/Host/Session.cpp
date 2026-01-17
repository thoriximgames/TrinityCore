#include "Session.h"
#include "SessionManager.h"
#include "../Engine/EntityManager.h"
#include "../Engine/DatabaseManager.h"
#include "../Shared/Cryptography/Crypto.h"
#include <iostream>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <atomic>

namespace MMO::Host {

Session::Session(tcp::socket socket)
    : _socket(std::move(socket)), _state(SessionState::Authenticating), _authenticated(false), _isWriting(false) 
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
            co_await HandlePacket(masterPacket);
        }
    } catch (...) {}
}

asio::awaitable<void> Session::HandlePacket(const Protocol::MasterPacket* packet) {
    auto type = packet->data_type();

    switch (type) {
        case Protocol::PacketType_LoginRequest: {
            auto req = packet->data_as_LoginRequest();
            std::string username = req->username()->str();
            std::string token = req->token() ? req->token()->str() : "";
            
            auto account = Engine::DatabaseManager::Instance().GetAccount(username);
            bool isValid = (account && token == account->password_hash);

            flatbuffers::FlatBufferBuilder fbb;
            if (!isValid) {
                auto msg = fbb.CreateString("Authentication failed.");
                auto res = Protocol::CreateLoginResponse(fbb, false, msg, 0, 0);
                auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_LoginResponse, res.Union());
                fbb.Finish(master);
                co_await Send(fbb);
                co_return;
            }

            _state = SessionState::SelectingRealm;
            _authenticated = true;
            _tempAccountId = account->id;
            _tempUsername = username;

            auto msg = fbb.CreateString("Authenticated.");
            Protocol::Vec3 zeroPos(0, 0, 0);
            auto res = Protocol::CreateLoginResponse(fbb, true, msg, account->id, &zeroPos);
            auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_LoginResponse, res.Union());
            fbb.Finish(master);
            co_await Send(fbb);
            break;
        }
        case Protocol::PacketType_RealmListRequest: {
            if (!_authenticated) co_return;
            flatbuffers::FlatBufferBuilder fbb;
            auto name = fbb.CreateString("Development Realm");
            auto realm = Protocol::CreateRealmInfo(fbb, 1, name, 0.1f, 1);
            std::vector<flatbuffers::Offset<Protocol::RealmInfo>> realmList = { realm };
            auto realmsVector = fbb.CreateVector(realmList);
            auto res = Protocol::CreateRealmListResponse(fbb, realmsVector);
            auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_RealmListResponse, res.Union());
            fbb.Finish(master);
            co_await Send(fbb);
            break;
        }
        case Protocol::PacketType_JoinRealmRequest: {
            if (!_authenticated || _state != SessionState::SelectingRealm) co_return;
            _state = SessionState::SelectingCharacter;
            std::cout << "[Session] User " << _tempUsername << " handoff to character selection." << std::endl;
            
            flatbuffers::FlatBufferBuilder fbb;
            auto res = Protocol::CreateJoinRealmResponse(fbb, true, fbb.CreateString("Success"));
            auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_JoinRealmResponse, res.Union());
            fbb.Finish(master);
            co_await Send(fbb);
            break;
        }
        case Protocol::PacketType_CharacterListRequest: {
            if (!_authenticated || _state != SessionState::SelectingCharacter) co_return;
            
            auto characters = Engine::DatabaseManager::Instance().GetCharacters(_tempAccountId);
            flatbuffers::FlatBufferBuilder fbb;
            std::vector<flatbuffers::Offset<Protocol::CharacterInfo>> charList;
            
            for (const auto& c : characters) {
                auto name = fbb.CreateString(c.name);
                charList.push_back(Protocol::CreateCharacterInfo(fbb, c.id, name, c.race, c.char_class, c.level));
            }
            
            auto charVector = fbb.CreateVector(charList);
            auto res = Protocol::CreateCharacterListResponse(fbb, charVector);
            auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_CharacterListResponse, res.Union());
            fbb.Finish(master);
            co_await Send(fbb);
            break;
        }
        case Protocol::PacketType_CreateCharacterRequest: {
            if (!_authenticated || _state != SessionState::SelectingCharacter) co_return;
            auto req = packet->data_as_CreateCharacterRequest();
            std::string name = req->name() ? req->name()->str() : "";
            
            flatbuffers::FlatBufferBuilder fbb;
            std::string error;
            if (name.length() < 2) error = "Name too short.";
            else if (Engine::DatabaseManager::Instance().IsCharacterNameTaken(name)) error = "Name already taken.";
            
            if (error.empty() && Engine::DatabaseManager::Instance().CreateCharacter(_tempAccountId, name, req->race(), req->character_class())) {
                auto msg = fbb.CreateString("Character created.");
                auto res = Protocol::CreateCreateCharacterResponse(fbb, true, msg);
                auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_CreateCharacterResponse, res.Union());
                fbb.Finish(master);
                co_await Send(fbb);
            } else {
                auto msg = fbb.CreateString(error.empty() ? "Creation failed." : error);
                auto res = Protocol::CreateCreateCharacterResponse(fbb, false, msg);
                auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_CreateCharacterResponse, res.Union());
                fbb.Finish(master);
                co_await Send(fbb);
            }
            break;
        }
        case Protocol::PacketType_DeleteCharacterRequest: {
            if (!_authenticated || _state != SessionState::SelectingCharacter) co_return;
            auto req = packet->data_as_DeleteCharacterRequest();
            std::string nameConf = req->name_confirmation() ? req->name_confirmation()->str() : "";
            
            auto charData = Engine::DatabaseManager::Instance().GetCharacter(req->character_id(), _tempAccountId);
            
            flatbuffers::FlatBufferBuilder fbb;
            bool success = false;
            std::string error;

            if (!charData) {
                error = "Character not found.";
            } else if (charData->name != nameConf) {
                error = "Name confirmation does not match.";
            }
            else {
                success = Engine::DatabaseManager::Instance().DeleteCharacter(req->character_id(), _tempAccountId);
                if (!success) error = "Database error.";
            }

            auto res = Protocol::CreateDeleteCharacterResponse(fbb, success, fbb.CreateString(error));
            auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_DeleteCharacterResponse, res.Union());
            fbb.Finish(master);
            co_await Send(fbb);
            break;
        }
        case Protocol::PacketType_SelectCharacterRequest: {
            if (!_authenticated || _state != SessionState::SelectingCharacter) co_return;
            auto req = packet->data_as_SelectCharacterRequest();
            auto charData = Engine::DatabaseManager::Instance().GetCharacter(req->character_id(), _tempAccountId);
            
            flatbuffers::FlatBufferBuilder fbb;
            if (charData) {
                _state = SessionState::InWorld;
                _player = std::make_shared<Engine::Player>(charData->id, charData->name);
                _player->SetPosition(charData->x, charData->y, charData->z);
                _player->SetRotation(charData->rotation);
                Engine::EntityManager::Instance().AddEntity(_player);
                std::cout << "[Session] Character " << charData->name << " entered world." << std::endl;

                auto res = Protocol::CreateSelectCharacterResponse(fbb, true, fbb.CreateString("Success"));
                auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_SelectCharacterResponse, res.Union());
                fbb.Finish(master);
                co_await Send(fbb);
            } else {
                auto res = Protocol::CreateSelectCharacterResponse(fbb, false, fbb.CreateString("Character not found."));
                auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_SelectCharacterResponse, res.Union());
                fbb.Finish(master);
                co_await Send(fbb);
            }
            break;
        }
        case Protocol::PacketType_RegisterRequest: {
            auto req = packet->data_as_RegisterRequest();
            std::string username = req->username() ? req->username()->str() : "";
            std::string email = req->email() ? req->email()->str() : "";
            std::string pw_hash = req->password_hash() ? req->password_hash()->str() : "";

            flatbuffers::FlatBufferBuilder fbb;
            std::string error;
            if (username.length() < 3) error = "Username too short.";
            else if (email.find('@') == std::string::npos) error = "Invalid email format.";
            else if (Engine::DatabaseManager::Instance().IsUsernameTaken(username)) error = "Username taken.";

            if (!error.empty()) {
                auto res = Protocol::CreateRegisterResponse(fbb, false, fbb.CreateString(error));
                auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_RegisterResponse, res.Union());
                fbb.Finish(master);
                co_await Send(fbb);
                co_return;
            }

            if (Engine::DatabaseManager::Instance().CreateAccount(username, email, pw_hash, "salt")) {
                auto res = Protocol::CreateRegisterResponse(fbb, true, fbb.CreateString("Success"));
                auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_RegisterResponse, res.Union());
                fbb.Finish(master);
                co_await Send(fbb);
            }
            break;
        }
        case Protocol::PacketType_MovementUpdate: {
            if (!_authenticated || !_player || _state != SessionState::InWorld) break;
            auto move = packet->data_as_MovementUpdate();
            _player->SetPosition(move->position()->x(), move->position()->y(), move->position()->z());
            _player->SetRotation(move->rotation());

            auto packetData = std::make_shared<std::vector<uint8_t>>();
            {
                flatbuffers::FlatBufferBuilder fbb;
                Protocol::Vec3 pos(_player->GetPosition().x(), _player->GetPosition().y(), _player->GetPosition().z());
                auto res = Protocol::CreateMovementUpdate(fbb, _player->GetId(), &pos, _player->GetRotation(), move->timestamp());
                auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_MovementUpdate, res.Union());
                fbb.Finish(master);
                uint32_t totalSize = fbb.GetSize();
                packetData->resize(sizeof(uint32_t) + totalSize);
                std::memcpy(packetData->data(), &totalSize, sizeof(uint32_t));
                std::memcpy(packetData->data() + sizeof(uint32_t), fbb.GetBufferPointer(), totalSize);
            }

            SessionManager::Instance().ForEachSession([&](std::shared_ptr<Session> other) {
                if (other.get() != this) other->QueuePacket(packetData);
            });
            break;
        }
        default: break;
    }
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
