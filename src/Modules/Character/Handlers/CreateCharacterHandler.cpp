#include "CreateCharacterHandler.h"
#include "../../../Host/Session.h"
#include "../../../Engine/DatabaseManager.h"

namespace MMO::Modules::Character::Handlers {

asio::awaitable<void> CreateCharacterHandler::Handle(
    const Shared::Architecture::TraceContext& ctx,
    const Protocol::MasterPacket* packet,
    std::shared_ptr<MMO::Host::Session> session
) {
    if (!session->_authenticated || session->_state != Host::SessionState::SelectingCharacter) {
        co_return;
    }

    auto req = packet->data_as_CreateCharacterRequest();
    std::string name = req->name() ? req->name()->str() : "";
    
    flatbuffers::FlatBufferBuilder fbb;
    std::string error;
    if (name.length() < 2) error = "Name too short.";
    else if (Engine::DatabaseManager::Instance().IsCharacterNameTaken(name)) error = "Name already taken.";
    
    if (error.empty() && Engine::DatabaseManager::Instance().CreateCharacter(session->_tempAccountId, name, req->race(), req->character_class())) {
        std::cout << "[Trace:" << ctx.TraceId << "] Character " << name << " created for account " << session->_tempAccountId << std::endl;
        auto msg = fbb.CreateString("Character created.");
        auto res = Protocol::CreateCreateCharacterResponse(fbb, true, msg);
        auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_CreateCharacterResponse, res.Union());
        fbb.Finish(master);
        co_await session->Send(fbb);
    } else {
        auto msg = fbb.CreateString(error.empty() ? "Creation failed." : error);
        auto res = Protocol::CreateCreateCharacterResponse(fbb, false, msg);
        auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_CreateCharacterResponse, res.Union());
        fbb.Finish(master);
        co_await session->Send(fbb);
    }
}

} // namespace MMO::Modules::Character::Handlers
