#pragma once
#include <unordered_map>
#include <memory>
#include "IPacketHandler.h"

namespace MMO::Shared::Architecture {

/**
 * @brief PacketDispatcher manages the routing of network packets to handlers.
 * It enforces the Deterministic Templating law by being the single point of entry.
 */
class PacketDispatcher {
public:
    static PacketDispatcher& Instance() {
        static PacketDispatcher instance;
        return instance;
    }

    /**
     * @brief Registers a handler for a specific packet type.
     */
    void RegisterHandler(Protocol::PacketType type, std::unique_ptr<IPacketHandler> handler) {
        _handlers[type] = std::move(handler);
    }

    /**
     * @brief Dispatches a packet to its registered handler.
     */
    asio::awaitable<void> Dispatch(
        const TraceContext& ctx,
        const Protocol::MasterPacket* packet,
        std::shared_ptr<MMO::Host::Session> session
    ) {
        auto it = _handlers.find(packet->data_type());
        if (it != _handlers.end()) {
            co_await it->second->Handle(ctx, packet, session);
        } else {
            // Handle unknown packet - Oracle will monitor for unhandled states
        }
    }

private:
    PacketDispatcher() = default;
    std::unordered_map<Protocol::PacketType, std::unique_ptr<IPacketHandler>> _handlers;
};

} // namespace MMO::Shared::Architecture
