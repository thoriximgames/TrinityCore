#include "DeleteCharacterHandler.h"
#include "../../../Host/Session.h"
#include "../../../Engine/DatabaseManager.h"

namespace MMO::Modules::Character::Handlers {

asio::awaitable<void> DeleteCharacterHandler::Handle(
    const Shared::Architecture::TraceContext& ctx,
    const Protocol::MasterPacket* packet,
    std::shared_ptr<MMO::Host::Session> session
) {
    if (!session->_authenticated || session->_state != Host::SessionState::SelectingCharacter) {
        co_return;
    }

    auto req = packet->data_as_DeleteCharacterRequest();
    std::string nameConf = req->name_confirmation() ? req->name_confirmation()->str() : "";
    
    auto charData = Engine::DatabaseManager::Instance().GetCharacter(req->character_id(), session->_tempAccountId);
    
    flatbuffers::FlatBufferBuilder fbb;
    bool success = false;
    std::string error;

    if (!charData) {
        error = "Character not found.";
    } else if (charData->name != nameConf) {
        error = "Name confirmation does not match.";
    }
    else {
        success = Engine::DatabaseManager::Instance().DeleteCharacter(req->character_id(), session->_tempAccountId);
        if (success) {
            std::cout << "[Trace:" << ctx.TraceId << "] Character " << req->character_id() << " deleted from account " << session->_tempAccountId << std::endl;
        }
        if (!success) error = "Database error.";
    }

    auto res = Protocol::CreateDeleteCharacterResponse(fbb, success, fbb.CreateString(error));
    auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_DeleteCharacterResponse, res.Union());
    fbb.Finish(master);
    co_await session->Send(fbb);
}

} // namespace MMO::Modules::Character::Handlers
