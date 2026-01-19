#include "RegisterHandler.h"
#include "../../../Host/Session.h"
#include "../../../Engine/DatabaseManager.h"

namespace MMO::Modules::Auth::Handlers {

asio::awaitable<void> RegisterHandler::Handle(
    const Shared::Architecture::TraceContext& ctx,
    const Protocol::MasterPacket* packet,
    std::shared_ptr<MMO::Host::Session> session
) {
    auto req = packet->data_as_RegisterRequest();
    std::string username = req->username() ? req->username()->str() : "";
    std::string email = req->email() ? req->email()->str() : "";
    std::string pw_hash = req->password_hash() ? req->password_hash()->str() : "";

    flatbuffers::FlatBufferBuilder fbb;
    std::string error;
    if (username.length() < 3) error = "Username too short.";
    else if (email.find('@') == std::string::npos) error = "Invalid email format.";
    else if (Engine::DatabaseManager::Instance().IsUsernameTaken(username)) error = "Username taken.";

    if (!error.empty()) {
        auto res = Protocol::CreateRegisterResponse(fbb, false, fbb.CreateString(error));
        auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_RegisterResponse, res.Union());
        fbb.Finish(master);
        co_await session->Send(fbb);
        co_return;
    }

    if (Engine::DatabaseManager::Instance().CreateAccount(username, email, pw_hash, "salt")) {
        std::cout << "[Trace:" << ctx.TraceId << "] User " << username << " registered successfully." << std::endl;
        auto res = Protocol::CreateRegisterResponse(fbb, true, fbb.CreateString("Success"));
        auto master = Protocol::CreateMasterPacket(fbb, Protocol::PacketType_RegisterResponse, res.Union());
        fbb.Finish(master);
        co_await session->Send(fbb);
    }
}

} // namespace MMO::Modules::Auth::Handlers
