#include "PacketParser.h"

PacketParser::PacketParser(const char* data, std::size_t size)
    : _data(data), _size(size), _offset(0)
{
}

PacketType PacketParser::ReadPacketType() {
    CheckSize(sizeof(uint8_t));
    uint8_t mask = *reinterpret_cast<const uint8_t*>(_data + _offset);
    _offset += sizeof(uint8_t);
    return static_cast<PacketType>(mask);
}

int PacketParser::ReadInt() {
    CheckSize(sizeof(int));
    int value;
    std::memcpy(&value, _data + _offset, sizeof(int));
    _offset += sizeof(int);
    return value;
}

std::size_t PacketParser::RemainingSize() const {
    return _size - _offset;
}

const char* PacketParser::CurrentPtr() const {
    return _data + _offset;
}

void PacketParser::CheckSize(std::size_t required) {
    if (_offset + required > _size) {
        throw std::runtime_error("PacketParser: Intento de lectura fuera de los límites del buffer.");
    }
}
