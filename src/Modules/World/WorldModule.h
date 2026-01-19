#pragma once
#include "../../Shared/Architecture/PacketDispatcher.h"
#include "Handlers/RealmListHandler.h"
#include "Handlers/JoinRealmHandler.h"
#include "Handlers/MovementHandler.h"

namespace MMO::Modules::World {

/**
 * @brief WorldModule is responsible for registering all world-related handlers.
 */
class WorldModule {
public:
    static void Initialize() {
        auto& dispatcher = Shared::Architecture::PacketDispatcher::Instance();
        
        dispatcher.RegisterHandler(
            Protocol::PacketType_RealmListRequest, 
            std::make_unique<Handlers::RealmListHandler>()
        );
        dispatcher.RegisterHandler(
            Protocol::PacketType_JoinRealmRequest, 
            std::make_unique<Handlers::JoinRealmHandler>()
        );
        dispatcher.RegisterHandler(
            Protocol::PacketType_MovementUpdate, 
            std::make_unique<Handlers::MovementHandler>()
        );
    }
};

} // namespace MMO::Modules::World
