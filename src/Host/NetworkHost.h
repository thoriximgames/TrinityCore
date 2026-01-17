#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <coroutine>
#include <asio.hpp>

namespace MMO::Host {

using asio::ip::tcp;

/**
 * @brief The NetworkHost class manages the low-level TCP listener
 * using C++20 coroutines for high performance.
 */
class NetworkHost {
public:
    NetworkHost(uint16_t port);
    ~NetworkHost();

    // Disable copy
    NetworkHost(const NetworkHost&) = delete;
    NetworkHost& operator=(const NetworkHost&) = delete;

    /**
     * @brief Starts the asynchronous IO loop.
     */
    void Run();

    /**
     * @brief Gracefully stops the host.
     */
    void Stop();

private:
    /**
     * @brief Coroutine to listen for incoming TCP connections.
     */
    asio::awaitable<void> Listen();

    asio::io_context _ioContext;
    uint16_t _port;
    bool _running;
};

} // namespace MMO::Host
