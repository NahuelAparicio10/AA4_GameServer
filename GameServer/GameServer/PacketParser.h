#pragma once
#include "Constants.h"
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <stdexcept>

class PacketParser {
public:
    PacketParser(const char* data, std::size_t size);

    PacketHeader ReadPacketType();
    int ReadInt();

    std::size_t RemainingSize() const;
    const char* CurrentPtr() const;

private:
    const char* _data;
    std::size_t _size;
    std::size_t _offset;

    void CheckSize(std::size_t required);
};
