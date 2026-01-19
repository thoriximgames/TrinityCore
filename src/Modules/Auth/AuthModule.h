#pragma once
#include "../../Shared/Architecture/PacketDispatcher.h"
#include "Handlers/LoginHandler.h"
#include "Handlers/RegisterHandler.h"

namespace MMO::Modules::Auth {

/**
 * @brief AuthModule is responsible for registering all authentication-related handlers.
 * Follows the Deterministic Templating law.
 */
class AuthModule {
public:
    static void Initialize() {
        auto& dispatcher = Shared::Architecture::PacketDispatcher::Instance();
        
        dispatcher.RegisterHandler(
            Protocol::PacketType_LoginRequest, 
            std::make_unique<Handlers::LoginHandler>()
        );

        dispatcher.RegisterHandler(
            Protocol::PacketType_RegisterRequest, 
            std::make_unique<Handlers::RegisterHandler>()
        );
    }
};

} // namespace MMO::Modules::Auth
