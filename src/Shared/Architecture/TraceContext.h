#pragma once
#include <cstdint>
#include <string>
#include <chrono>

namespace MMO::Shared::Architecture {

/**
 * @brief TraceContext implements the Trace Object Mandate (AOP v3.0).
 * It carries request history, identity, and timing across the system.
 */
struct TraceContext {
    uint64_t TraceId;      // Unique ID for this specific request flow
    uint64_t AccountId;    // The actor performing the action (0 if anonymous)
    uint64_t Timestamp;    // When the request entered the system
    std::string Origin;    // The module or service that originated the request

    static TraceContext Create(uint64_t accountId, const std::string& origin) {
        return {
            static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count()),
            accountId,
            static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count()),
            origin
        };
    }
};

} // namespace MMO::Shared::Architecture
