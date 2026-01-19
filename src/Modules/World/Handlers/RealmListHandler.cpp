#include "RealmListHandler.h"
#include "../../../Host/Session.h"
#include <vector>
#include <iostream>

namespace MMO::Modules::World::Handlers {

asio::awaitable<void> RealmListHandler::Handle(
    const Shared::Architecture::TraceContext& ctx,
    [[maybe_unused]] const Protocol::MasterPacket* packet,
    std::shared_ptr<MMO::Host::Session> session
) {
    if (!session->_authenticated) co_return;

    std::cout << "[Trace:" << ctx.TraceId << "] Realm list requested by " << session->_tempUsername << std::endl;

    flatbuffers::FlatBufferBuilder fbb;
    auto name = fbb.CreateString("Development Realm");
    auto realm = Protocol::CreateRealmInfo(fbb, 1, name, 0.1f, 1);
    std::vector<flatbuffers::Offset<Protocol::RealmInfo>> realmList = { realm };
    
    auto realmsVector = fbb.CreateVector(realmList);
    auto res = Protocol::CreateRealmListResponse(fbb, realmsVector);
    auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_RealmListResponse, res.Union());
    fbb.Finish(master);
    co_await session->Send(fbb);
}

} // namespace MMO::Modules::World::Handlers
