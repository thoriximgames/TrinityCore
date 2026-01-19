#include "CharacterListHandler.h"
#include "../../../Host/Session.h"
#include "../../../Engine/DatabaseManager.h"
#include <vector>

namespace MMO::Modules::Character::Handlers {

asio::awaitable<void> CharacterListHandler::Handle(
    const Shared::Architecture::TraceContext& ctx,
    [[maybe_unused]] const Protocol::MasterPacket* packet,
    std::shared_ptr<MMO::Host::Session> session
) {
    if (!session->_authenticated || session->_state != Host::SessionState::SelectingCharacter) {
        co_return;
    }
    
    std::cout << "[Trace:" << ctx.TraceId << "] Fetching character list for account " << session->_tempAccountId << std::endl;
    
    auto characters = Engine::DatabaseManager::Instance().GetCharacters(session->_tempAccountId);
    flatbuffers::FlatBufferBuilder fbb;
    std::vector<flatbuffers::Offset<Protocol::CharacterInfo>> charList;
    
    for (const auto& c : characters) {
        auto name = fbb.CreateString(c.name);
        charList.push_back(Protocol::CreateCharacterInfo(fbb, c.id, name, c.race, c.char_class, c.level));
    }
    
    auto charVector = fbb.CreateVector(charList);
    auto res = Protocol::CreateCharacterListResponse(fbb, charVector);
    auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_CharacterListResponse, res.Union());
    fbb.Finish(master);
    co_await session->Send(fbb);
}

} // namespace MMO::Modules::Character::Handlers
