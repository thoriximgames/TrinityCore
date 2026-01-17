#pragma once
#include <thread>
#include <atomic>
#include <string>
#include <iostream>
#include "RedisManager.h"
#include "DatabaseManager.h"

namespace MMO::Engine {

/**
 * @brief Background worker that drains the Redis queue and persists to PostgreSQL.
 * This is the "Tier 3" of our persistence strategy.
 */
class PersistenceWorker {
public:
    static PersistenceWorker& Instance() {
        static PersistenceWorker instance;
        return instance;
    }

    void Start() {
        if (_running) return;
        _running = true;
        _workerThread = std::thread(&PersistenceWorker::WorkLoop, this);
        std::cout << "[PersistenceWorker] Started Tier 3 Write-Behind." << std::endl;
    }

    void Stop() {
        _running = false;
        if (_workerThread.joinable()) _workerThread.join();
        std::cout << "[PersistenceWorker] Stopped." << std::endl;
    }

private:
    PersistenceWorker() : _running(false) {}

    void WorkLoop() {
        while (_running) {
            std::string task = RedisManager::Instance().PopPersistenceTask();
            if (task.empty()) continue;

            try {
                // Simple parser for our current demo task format
                // In production, use a JSON library like nlohmann/json
                ProcessTask(task);
            } catch (const std::exception& e) {
                std::cerr << "[PersistenceWorker] Error processing task: " << e.what() << std::endl;
            }
        }
    }

    void ProcessTask(const std::string& task) {
        // Expected format for position sync: "POS|charId|x|y|z|rot"
        if (task.substr(0, 4) == "POS|") {
            // Very simple parsing for the prototype
            size_t p1 = task.find('|', 4);
            size_t p2 = task.find('|', p1 + 1);
            size_t p3 = task.find('|', p2 + 1);
            size_t p4 = task.find('|', p3 + 1);

            uint64_t charId = std::stoull(task.substr(4, p1 - 4));
            float x = std::stof(task.substr(p1 + 1, p2 - p1 - 1));
            float y = std::stof(task.substr(p2 + 1, p3 - p2 - 1));
            float z = std::stof(task.substr(p3 + 1, p4 - p3 - 1));
            float rot = std::stof(task.substr(p4 + 1));

            // Perform the SQL Update
            DatabaseManager::Instance().UpdateCharacterPosition(charId, x, y, z, rot);
        }
    }

    std::thread _workerThread;
    std::atomic<bool> _running;
};

} // namespace MMO::Engine
