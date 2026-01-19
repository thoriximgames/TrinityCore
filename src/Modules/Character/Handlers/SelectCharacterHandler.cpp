#include "SelectCharacterHandler.h"
#include "../../../Host/Session.h"
#include "../../../Engine/DatabaseManager.h"
#include "../../../Engine/EntityManager.h"
#include <iostream>

namespace MMO::Modules::Character::Handlers {

asio::awaitable<void> SelectCharacterHandler::Handle(
    const Shared::Architecture::TraceContext& ctx,
    const Protocol::MasterPacket* packet,
    std::shared_ptr<MMO::Host::Session> session
) {
    if (!session->_authenticated || session->_state != Host::SessionState::SelectingCharacter) {
        co_return;
    }

    auto req = packet->data_as_SelectCharacterRequest();
    auto charData = Engine::DatabaseManager::Instance().GetCharacter(req->character_id(), session->_tempAccountId);
    
    flatbuffers::FlatBufferBuilder fbb;
    if (charData) {
        session->_state = Host::SessionState::InWorld;
        session->_player = std::make_shared<Engine::Player>(charData->id, charData->name);
        session->_player->SetPosition(charData->x, charData->y, charData->z);
        session->_player->SetRotation(charData->rotation);
        Engine::EntityManager::Instance().AddEntity(session->_player);
        
        std::cout << "[Trace:" << ctx.TraceId << "] Character " << charData->name << " entered world." << std::endl;

        auto res = Protocol::CreateSelectCharacterResponse(fbb, true, fbb.CreateString("Success"));
        auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_SelectCharacterResponse, res.Union());
        fbb.Finish(master);
        co_await session->Send(fbb);
    } else {
        auto res = Protocol::CreateSelectCharacterResponse(fbb, false, fbb.CreateString("Character not found."));
        auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_SelectCharacterResponse, res.Union());
        fbb.Finish(master);
        co_await session->Send(fbb);
    }
}

} // namespace MMO::Modules::Character::Handlers
