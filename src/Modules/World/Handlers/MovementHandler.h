#pragma once
#include "../../../Shared/Architecture/IPacketHandler.h"

namespace MMO::Modules::World::Handlers {

/**
 * @brief Handles movement updates and broadcasts them to other players.
 */
class MovementHandler : public Shared::Architecture::IPacketHandler {
public:
    asio::awaitable<void> Handle(
        const Shared::Architecture::TraceContext& ctx,
        const Protocol::MasterPacket* packet,
        std::shared_ptr<MMO::Host::Session> session
    ) override;
};

} // namespace MMO::Modules::World::Handlers
