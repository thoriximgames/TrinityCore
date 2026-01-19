#pragma once
#include "../../../Shared/Architecture/IPacketHandler.h"

namespace MMO::Modules::World::Handlers {

/**
 * @brief Handles requests for the realm list.
 */
class RealmListHandler : public Shared::Architecture::IPacketHandler {
public:
    asio::awaitable<void> Handle(
        const Shared::Architecture::TraceContext& ctx,
        const Protocol::MasterPacket* packet,
        std::shared_ptr<MMO::Host::Session> session
    ) override;
};

} // namespace MMO::Modules::World::Handlers
