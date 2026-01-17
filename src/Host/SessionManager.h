#pragma once
#include <unordered_set>
#include <memory>
#include <mutex>

namespace MMO::Host {

class Session;

/**
 * @brief Thread-safe registry for all active sessions.
 * Used for broadcasting updates to all connected clients.
 */
class SessionManager {
public:
    static SessionManager& Instance() {
        static SessionManager instance;
        return instance;
    }

    void AddSession(std::shared_ptr<Session> session) {
        std::lock_guard lock(_mutex);
        _sessions.insert(session);
    }

    void RemoveSession(std::shared_ptr<Session> session) {
        std::lock_guard lock(_mutex);
        _sessions.erase(session);
    }

    template<typename F>
    void ForEachSession(F&& func) {
        std::lock_guard lock(_mutex);
        for (auto& session : _sessions) {
            func(session);
        }
    }

private:
    SessionManager() = default;
    std::unordered_set<std::shared_ptr<Session>> _sessions;
    std::mutex _mutex;
};

} // namespace MMO::Host
