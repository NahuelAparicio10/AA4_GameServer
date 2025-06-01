#pragma once
#include <SFML/Network.hpp>
#include <vector>
#include <string>
#include <sstream>

enum class MatchType { NORMAL, RANKED };

struct ClientMatchInfo {
    sf::IpAddress ip;
    unsigned short port;
    unsigned int playerID = 0;
};

struct StartMatchData {
    unsigned int matchID;
    MatchType type;
    std::vector<ClientMatchInfo> players;
};
// -- Serializa formato: matchID:type:ip:port:playerID:ip:port:playerID...
inline std::string SerializeMatch(const StartMatchData& data) {
    std::ostringstream ss;
    ss << data.matchID << ":" << (data.type == MatchType::RANKED ? "RANKED" : "NORMAL");
    for (const auto& p : data.players)
        ss << ":" << p.ip.toString() << ":" << p.port << ":" << p.playerID;
    return ss.str();
}

// -- Deserializa el mismo formato
inline StartMatchData DeserializeMatch(const std::string& str) {
    StartMatchData data;
    std::istringstream ss(str);
    std::string token;

    std::getline(ss, token, ':');
    data.matchID = std::stoi(token);

    std::getline(ss, token, ':');
    data.type = (token == "RANKED") ? MatchType::RANKED : MatchType::NORMAL;

    while (std::getline(ss, token, ':')) {
        std::optional<sf::IpAddress> ip = sf::IpAddress::resolve(token);
        std::getline(ss, token, ':');
        unsigned short port = static_cast<unsigned short>(std::stoi(token));
        std::getline(ss, token, ':');
        unsigned int playerID = static_cast<unsigned int>(std::stoi(token));

        data.players.push_back({ ip.value(), port, playerID });
    }

    return data;
}