#pragma once
#include "../../../Shared/Architecture/IPacketHandler.h"

namespace MMO::Modules::Auth::Handlers {

/**
 * @brief LoginHandler handles authentication requests.
 * Fits into the REPR pattern as an Endpoint.
 */
class LoginHandler : public Shared::Architecture::IPacketHandler {
public:
    asio::awaitable<void> Handle(
        const Shared::Architecture::TraceContext& ctx,
        const Protocol::MasterPacket* packet,
        std::shared_ptr<MMO::Host::Session> session
    ) override;
};

} // namespace MMO::Modules::Auth::Handlers
