#pragma once

#include <vector>
#include <cstdint>
#include "OpCodes.h"

namespace MMO::Networking {

#pragma pack(push, 1)
/**
 * @brief World Packet Header (WoW 3.3.5a)
 * Server to Client: 2 bytes length, 2 bytes opcode
 * Client to Server: 2 bytes length, 4 bytes opcode
 */
struct WorldPacketHeader {
    uint16_t size;
    uint32_t opcode;
};
#pragma pack(pop)

/**
 * @brief Modern representation of a Network Packet
 */
struct Packet {
    uint32_t opcode;
    std::vector<uint8_t> data;

    Packet(uint32_t op) : opcode(op) {}
    Packet(uint32_t op, std::vector<uint8_t>&& d) : opcode(op), data(std::move(d)) {}
};

} // namespace MMO::Networking
