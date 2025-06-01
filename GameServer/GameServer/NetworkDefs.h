#pragma once
#include <SFML/Network.hpp>
#include <cstdint>
#include <string>

enum PacketHeader : uint8_t {
    NORMAL = 0b00000001,
    URGENT = 0b00000010,
    CRITICAL = 0b00000100
};

enum class PacketType : uint8_t {
    INVALID = 0,
    FIND_MATCH = 1,
    ACK_MATCH_FOUND = 2,
    CANCEL_SEARCH = 3,
    MATCH_FOUND = 4,
    VERSION = 5,
    UPDATE = 6,
    OK = 7,
    UPDATE_MAP = 8,
    SEARCH_ACK = 9,
    JOIN_GAME = 10,
    CREATE_MATCH = 11,
    MATCH_UNIQUE = 12,
    MATCH_USED = 13

};


struct RawPacketJob {
    uint8_t headerMask;
    PacketType type;
    std::string content;
    std::optional<sf::IpAddress> sender;
    unsigned short port;
};

// Función para crear un datagrama listo para enviar
inline std::size_t CreateRawDatagram(uint8_t headerMask, PacketType type, const std::string& content, char* outBuffer)
{
    outBuffer[0] = headerMask;
    outBuffer[1] = static_cast<uint8_t>(type);
    std::memcpy(outBuffer + 2, content.data(), content.size());
    return 2 + content.size();
}

inline void SendDatagram(sf::UdpSocket& socket, PacketHeader header, PacketType type, const std::string& content, const sf::IpAddress& ip, unsigned short port)
{
    char buffer[1024];
    std::size_t size = CreateRawDatagram(static_cast<uint8_t>(header), type, content, buffer);
    socket.send(buffer, size, ip, port);
}

// Función para parsear datagramas binarios
inline bool ParseRawDatagram(const char* data, std::size_t size, RawPacketJob& out, const sf::IpAddress& ip, unsigned short port)
{
    if (size < 2) return false;

    out.headerMask = static_cast<uint8_t>(data[0]);
    out.type = static_cast<PacketType>(data[1]);

    if (out.type == PacketType::INVALID) return false;

    out.content = std::string(data + 2, size - 2);
    out.sender = ip;
    out.port = port;

    return true;
}

