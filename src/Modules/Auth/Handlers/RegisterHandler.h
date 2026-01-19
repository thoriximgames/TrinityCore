#pragma once
#include "../../../Shared/Architecture/IPacketHandler.h"

namespace MMO::Modules::Auth::Handlers {

/**
 * @brief Handles account registration requests.
 */
class RegisterHandler : public Shared::Architecture::IPacketHandler {
public:
    asio::awaitable<void> Handle(
        const Shared::Architecture::TraceContext& ctx,
        const Protocol::MasterPacket* packet,
        std::shared_ptr<MMO::Host::Session> session
    ) override;
};

} // namespace MMO::Modules::Auth::Handlers
