#pragma once
#include <cstdint>

inline unsigned short GameServerPort = 55000;

enum PacketType : uint8_t {

    NORMAL= 0b00000000,
    URGENT = 0b00000001,
    CRITICAL = 0b00000010,
    START_MATCH = 0b00000100,
    PLAYER_READY = 0b00001000

};
