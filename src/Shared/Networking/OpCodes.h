#pragma once

#include <cstdint>

namespace MMO::Networking {

/**
 * @brief AuthServer OpCodes (WoW 3.3.5a)
 */
enum class AuthOpCode : uint8_t {
    LOGON_CHALLENGE    = 0x00,
    LOGON_PROOF        = 0x01,
    RECONNECT_CHALLENGE = 0x02,
    RECONNECT_PROOF     = 0x03,
    REALM_LIST         = 0x10,
    XFER_INIT          = 0x30,
    XFER_DATA          = 0x31
};

/**
 * @brief WorldServer OpCodes (WoW 3.3.5a)
 */
enum class WorldOpCode : uint32_t {
    CMSG_CHAR_ENUM              = 0x0037,
    SMSG_CHAR_ENUM              = 0x003B,
    CMSG_PLAYER_LOGIN           = 0x003D,
    SMSG_CHARACTER_LOGIN_FAILED  = 0x0041,
    SMSG_LOGIN_VERIFY_WORLD     = 0x0236,
    CMSG_AUTH_SESSION           = 0x01ED,
    SMSG_AUTH_CHALLENGE         = 0x01EC,
    SMSG_AUTH_RESPONSE          = 0x01EE,
    
    // Movement Opcodes
    MSG_MOVE_START_FORWARD      = 0x00B5,
    MSG_MOVE_HEARTBEAT          = 0x00EE,
    
    // Add more as harvested from Reference...
};

} // namespace MMO::Networking
