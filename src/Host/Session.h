#pragma once

#include <asio.hpp>
#include <memory>
#include <vector>
#include <deque>
#include <mutex>
#include <string>
#include "../Shared/Protocol/MMO_generated.h"
#include "../Engine/Entity.h"

namespace MMO::Host {

using asio::ip::tcp;

enum class SessionState {
    Authenticating,
    SelectingRealm,
    SelectingCharacter,
    InWorld
};

/**
 * @brief Represents a single connected client session.
 * Manages asynchronous reading/writing of FlatBuffer packets.
 */
class Session : public std::enable_shared_from_this<Session> {
public:
    explicit Session(tcp::socket socket);
    ~Session();

    /**
     * @brief The main coroutine that drives the session lifecycle.
     */
    asio::awaitable<void> Process();

    /**
     * @brief Sends a FlatBuffer packet to the client (waitable).
     */
    asio::awaitable<void> Send(flatbuffers::FlatBufferBuilder& fbb);

    /**
     * @brief Queue a packet for sending (thread-safe, for broadcasting).
     */
    void QueuePacket(std::shared_ptr<std::vector<uint8_t>> packetData);

    // Session State (Public for Module Handlers)
    SessionState _state;
    bool _authenticated;
    std::shared_ptr<Engine::Player> _player;

    // For Handoff between Login and World
    uint64_t _tempAccountId;
    std::string _tempUsername;

private:
    /**
     * @brief Handles a specific packet type.
     */
    asio::awaitable<void> HandlePacket(const Protocol::MasterPacket* packet);

    /**
     * @brief Internal loop to process the write queue.
     */
    asio::awaitable<void> WriteLoop();

    tcp::socket _socket;

    // Thread-safe write queue
    std::deque<std::shared_ptr<std::vector<uint8_t>>> _writeQueue;
    std::mutex _writeMutex;
    bool _isWriting;
};

} // namespace MMO::Host
