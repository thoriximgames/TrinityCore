#pragma once
#include "../../../Shared/Architecture/IPacketHandler.h"

namespace MMO::Modules::Character::Handlers {

/**
 * @brief Handles requests to delete a character.
 */
class DeleteCharacterHandler : public Shared::Architecture::IPacketHandler {
public:
    asio::awaitable<void> Handle(
        const Shared::Architecture::TraceContext& ctx,
        const Protocol::MasterPacket* packet,
        std::shared_ptr<MMO::Host::Session> session
    ) override;
};

} // namespace MMO::Modules::Character::Handlers
