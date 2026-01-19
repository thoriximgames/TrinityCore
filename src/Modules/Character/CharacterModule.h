#pragma once
#include "../../Shared/Architecture/PacketDispatcher.h"
#include "Handlers/CharacterListHandler.h"
#include "Handlers/CreateCharacterHandler.h"
#include "Handlers/DeleteCharacterHandler.h"
#include "Handlers/SelectCharacterHandler.h"

namespace MMO::Modules::Character {

/**
 * @brief CharacterModule is responsible for registering all character-related handlers.
 */
class CharacterModule {
public:
    static void Initialize() {
        auto& dispatcher = Shared::Architecture::PacketDispatcher::Instance();
        
        dispatcher.RegisterHandler(
            Protocol::PacketType_CharacterListRequest, 
            std::make_unique<Handlers::CharacterListHandler>()
        );
        dispatcher.RegisterHandler(
            Protocol::PacketType_CreateCharacterRequest, 
            std::make_unique<Handlers::CreateCharacterHandler>()
        );
        dispatcher.RegisterHandler(
            Protocol::PacketType_DeleteCharacterRequest, 
            std::make_unique<Handlers::DeleteCharacterHandler>()
        );
        dispatcher.RegisterHandler(
            Protocol::PacketType_SelectCharacterRequest, 
            std::make_unique<Handlers::SelectCharacterHandler>()
        );
    }
};

} // namespace MMO::Modules::Character
