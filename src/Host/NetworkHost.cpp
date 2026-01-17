#include "NetworkHost.h"
#include "Session.h"
#include "SessionManager.h"
#include <thread>
#include <chrono>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>

namespace MMO::Host {

NetworkHost::NetworkHost(uint16_t port) 
    : _port(port), _running(false) 
{
}

NetworkHost::~NetworkHost() {
    if (_running) {
        Stop();
    }
}

void NetworkHost::Run() {
    _running = true;
    
    try {
        // Start the listener coroutine
        asio::co_spawn(_ioContext, Listen(), asio::detached);

        std::cout << "[Host] Modern MMO Backend listening on port " << _port << "..." << std::endl;
        
        // io_context::run() blocks until all work is done
        _ioContext.run();

    } catch (const std::exception& e) {
        std::cerr << "[Host] Exception in IO loop: " << e.what() << std::endl;
    }
}

void NetworkHost::Stop() {
    std::cout << "[Host] Shutting down..." << std::endl;
    _running = false;
    _ioContext.stop();
}

asio::awaitable<void> NetworkHost::Listen() {
    auto executor = co_await asio::this_coro::executor;
    tcp::acceptor acceptor(executor, {tcp::v4(), _port});

    while (_running) {
        // Wait for a new connection
        tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
        
        std::cout << "[Host] New connection from: " << socket.remote_endpoint() << std::endl;

        // Create a new session and spawn its processing coroutine
        auto session = std::make_shared<Session>(std::move(socket));
        
        // Register session
        SessionManager::Instance().AddSession(session);

        asio::co_spawn(executor, [session]() -> asio::awaitable<void> {
            try {
                co_await session->Process();
            } catch (...) {
                // Handle or log session-specific errors
            }
            // Unregister session when it finishes
            SessionManager::Instance().RemoveSession(session);
        }, asio::detached);
    }
}

} // namespace MMO::Host