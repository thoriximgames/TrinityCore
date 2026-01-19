#pragma once
#include <asio.hpp>
#include <memory>
#include "TraceContext.h"
#include "../Protocol/MMO_generated.h"

namespace MMO::Host { class Session; }

namespace MMO::Shared::Architecture {

/**
 * @brief IPacketHandler is the "Restraining Base" for all packet logic.
 * Invalid states are prevented by the strict interface contract.
 */
class IPacketHandler {
public:
    virtual ~IPacketHandler() = default;

    /**
     * @brief The "Hook": Executes the handler logic.
     * @param ctx The trace context for bitemporal tracing.
     * @param packet The raw master packet from the network.
     * @param session The shared session that received the packet.
     */
    virtual asio::awaitable<void> Handle(
        const TraceContext& ctx,
        const Protocol::MasterPacket* packet,
        std::shared_ptr<MMO::Host::Session> session
    ) = 0;
};

} // namespace MMO::Shared::Architecture
