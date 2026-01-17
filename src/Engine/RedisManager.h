#pragma once
#include <hiredis/hiredis.h>
#include <string>
#include <iostream>
#include <vector>
#include <memory>

namespace MMO::Engine {

/**
 * @brief Manages the high-speed Warm Storage layer (Redis).
 */
class RedisManager {
public:
    static RedisManager& Instance() {
        static RedisManager instance;
        return instance;
    }

    bool Connect(const std::string& host, int port) {
        struct timeval timeout = { 1, 500000 }; // 1.5 seconds
        _context.reset(redisConnectWithTimeout(host.c_str(), port, timeout));

        if (_context == nullptr || _context->err) {
            if (_context) {
                std::cerr << "[Redis] Connection error: " << _context->errstr << std::endl;
            } else {
                std::cerr << "[Redis] Connection error: can't allocate redis context" << std::endl;
            }
            return false;
        }

        std::cout << "[Redis] Connected to Warm Storage at " << host << ":" << port << std::endl;
        return true;
    }

    /**
     * @brief Sets a character's live position in Redis.
     */
    void SetCharacterPosition(uint64_t charId, float x, float y, float z, float rot) {
        if (!_context) return;

        const std::string key = "char:" + std::to_string(charId) + ":pos";
        const std::string value = std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z) + "," + std::to_string(rot);

        auto reply = (redisReply*)redisCommand(_context.get(), "SET %s %s", key.c_str(), value.c_str());
        if (reply) freeReplyObject(reply);
    }

    /**
     * @brief Pushes a critical update to the write-behind queue.
     */
    void EnqueueSaveTask(const std::string& taskJson) {
        if (!_context) return;

        auto reply = (redisReply*)redisCommand(_context.get(), "RPUSH persistence_queue %s", taskJson.c_str());
        if (reply) freeReplyObject(reply);
    }

    /**
     * @brief Pops a task from the write-behind queue (blocking with timeout).
     */
    std::string PopPersistenceTask() {
        if (!_context) return "";

        // BLPOP returns [key, value]
        auto reply = (redisReply*)redisCommand(_context.get(), "BLPOP persistence_queue 1");
        if (reply && reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
            std::string task = reply->element[1]->str;
            freeReplyObject(reply);
            return task;
        }
        if (reply) freeReplyObject(reply);
        return "";
    }

private:
    RedisManager() = default;
    
    struct RedisContextDeleter {
        void operator()(redisContext* c) { if (c) redisFree(c); }
    };
    std::unique_ptr<redisContext, RedisContextDeleter> _context;
};

} // namespace MMO::Engine
