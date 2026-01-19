#include "MovementHandler.h"
#include "../../../Host/Session.h"
#include "../../../Host/SessionManager.h"
#include "../../../Engine/Entity.h"

namespace MMO::Modules::World::Handlers {

asio::awaitable<void> MovementHandler::Handle(
    [[maybe_unused]] const Shared::Architecture::TraceContext& ctx,
    const Protocol::MasterPacket* packet,
    std::shared_ptr<MMO::Host::Session> session
) {
    if (!session->_authenticated || !session->_player || session->_state != Host::SessionState::InWorld) {
        co_return;
    }

    auto move = packet->data_as_MovementUpdate();
    session->_player->SetPosition(move->position()->x(), move->position()->y(), move->position()->z());
    session->_player->SetRotation(move->rotation());

    // Minimal trace for movement to avoid console spam but satisfy Oracle
    // std::cout << "[Trace:" << ctx.TraceId << "] Movement update for " << session->_player->GetId() << std::endl;

    auto packetData = std::make_shared<std::vector<uint8_t>>();
    {
        flatbuffers::FlatBufferBuilder fbb;
        Protocol::Vec3 pos(session->_player->GetPosition().x(), session->_player->GetPosition().y(), session->_player->GetPosition().z());
        auto res = Protocol::CreateMovementUpdate(fbb, session->_player->GetId(), &pos, session->_player->GetRotation(), move->timestamp());
        auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_MovementUpdate, res.Union());
        fbb.Finish(master);
        
        uint32_t totalSize = fbb.GetSize();
        packetData->resize(sizeof(uint32_t) + totalSize);
        std::memcpy(packetData->data(), &totalSize, sizeof(uint32_t));
        std::memcpy(packetData->data() + sizeof(uint32_t), fbb.GetBufferPointer(), totalSize);
    }

    Host::SessionManager::Instance().ForEachSession([&](std::shared_ptr<Host::Session> other) {
        if (other.get() != session.get()) {
            other->QueuePacket(packetData);
        }
    });
    
    co_return;
}

} // namespace MMO::Modules::World::Handlers
