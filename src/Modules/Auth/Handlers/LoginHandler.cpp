#include "LoginHandler.h"
#include "../../../Host/Session.h"
#include "../../../Engine/DatabaseManager.h"
#include <iostream>

namespace MMO::Modules::Auth::Handlers {

asio::awaitable<void> HandleInternal(
    const Shared::Architecture::TraceContext& ctx,
    const Protocol::LoginRequest* req,
    std::shared_ptr<MMO::Host::Session> session
) {
    std::string username = req->username()->str();
    std::string token = req->token() ? req->token()->str() : "";
    
    auto account = Engine::DatabaseManager::Instance().GetAccount(username);
    bool isValid = (account && token == account->password_hash);

    flatbuffers::FlatBufferBuilder fbb;
    if (!isValid) {
        auto msg = fbb.CreateString("Authentication failed.");
        auto res = Protocol::CreateLoginResponse(fbb, false, msg, 0, 0);
        auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_LoginResponse, res.Union());
        fbb.Finish(master);
        co_await session->Send(fbb);
        co_return;
    }

    // Success logic
    session->_authenticated = true;
    session->_state = Host::SessionState::SelectingRealm;
    session->_tempAccountId = account->id;
    session->_tempUsername = username;

    auto msg = fbb.CreateString("Authenticated.");
    Protocol::Vec3 zeroPos(0, 0, 0);
    auto res = Protocol::CreateLoginResponse(fbb, true, msg, account->id, &zeroPos);
    auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_LoginResponse, res.Union());
    fbb.Finish(master);
    
    std::cout << "[Trace:" << ctx.TraceId << "] User " << username << " authenticated successfully." << std::endl;
    co_await session->Send(fbb);
}

asio::awaitable<void> LoginHandler::Handle(
    const Shared::Architecture::TraceContext& ctx,
    const Protocol::MasterPacket* packet,
    std::shared_ptr<MMO::Host::Session> session
) {
    auto req = packet->data_as_LoginRequest();
    if (req) {
        co_await HandleInternal(ctx, req, session);
    }
}

} // namespace MMO::Modules::Auth::Handlers
