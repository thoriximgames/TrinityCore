#include "JoinRealmHandler.h"
#include "../../../Host/Session.h"
#include <iostream>

namespace MMO::Modules::World::Handlers {

asio::awaitable<void> JoinRealmHandler::Handle(
    const Shared::Architecture::TraceContext& ctx,
    [[maybe_unused]] const Protocol::MasterPacket* packet,
    std::shared_ptr<MMO::Host::Session> session
) {
    if (!session->_authenticated || session->_state != Host::SessionState::SelectingRealm) {
        co_return;
    }

    session->_state = Host::SessionState::SelectingCharacter;
    std::cout << "[Trace:" << ctx.TraceId << "] User " << session->_tempUsername << " handoff to character selection." << std::endl;
    
    flatbuffers::FlatBufferBuilder fbb;
    auto res = Protocol::CreateJoinRealmResponse(fbb, true, fbb.CreateString("Success"));
    auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_JoinRealmResponse, res.Union());
    fbb.Finish(master);
    co_await session->Send(fbb);
}

} // namespace MMO::Modules::World::Handlers
