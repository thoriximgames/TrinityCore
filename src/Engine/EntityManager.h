#pragma once
#include <unordered_map>
#include <memory>
#include <shared_mutex>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include "Entity.h"
#include "RedisManager.h"

namespace MMO::Engine {

/**
 * @brief Manages all entities in the world.
 * Currently a simple global registry, will be expanded for spatial partitioning.
 */
class EntityManager {
public:
    static EntityManager& Instance() {
        static EntityManager instance;
        return instance;
    }

    void StartPersistenceThread() {
        _running = true;
        _syncThread = std::thread([this]() {
            while (_running) {
                SyncToWarmStorage();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 1Hz Sync
            }
        });
    }

    void StopPersistenceThread() {
        _running = false;
        if (_syncThread.joinable()) _syncThread.join();
    }

    /**
     * @brief Adds an entity to the world.
     */
    void AddEntity(std::shared_ptr<Entity> entity) {
        std::unique_lock lock(_mutex);
        _entities[entity->GetId()] = entity;
    }

    /**
     * @brief Removes an entity by ID.
     */
    void RemoveEntity(uint64_t id) {
        std::unique_lock lock(_mutex);
        _entities.erase(id);
    }

    /**
     * @brief Gets an entity by ID.
     */
    std::shared_ptr<Entity> GetEntity(uint64_t id) {
        std::shared_lock lock(_mutex);
        auto it = _entities.find(id);
        if (it != _entities.end()) {
            return it->second;
        }
        return nullptr;
    }

    /**
     * @brief Returns a copy of all entity pointers (for broadcasting/iteration).
     */
    std::vector<std::shared_ptr<Entity>> GetAllEntities() {
        std::shared_lock lock(_mutex);
        std::vector<std::shared_ptr<Entity>> result;
        result.reserve(_entities.size());
        for (const auto& [id, entity] : _entities) {
            result.push_back(entity);
        }
        return result;
    }

private:
    EntityManager() : _running(false) {}
    
    void SyncToWarmStorage() {
        auto entities = GetAllEntities();
        for (auto& entity : entities) {
            if (entity->GetType() == Protocol::EntityType_Player && entity->IsDirty()) {
                auto pos = entity->GetPosition();
                RedisManager::Instance().SetCharacterPosition(entity->GetId(), pos.x(), pos.y(), pos.z(), entity->GetRotation());
                entity->ClearDirty();
            }
        }
    }

    std::unordered_map<uint64_t, std::shared_ptr<Entity>> _entities;
    mutable std::shared_mutex _mutex;
    std::thread _syncThread;
    std::atomic<bool> _running;
};

} // namespace MMO::Engine