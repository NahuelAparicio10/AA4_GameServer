#pragma once
#include <SFML/Network.hpp>
#include <vector>
#include <string>
#include <sstream>

enum class MatchType { NORMAL, RANKED };

struct ClientMatchInfo {
    sf::IpAddress ip = sf::IpAddress::Any;
    unsigned short port;
    unsigned int playerID = 0;
};

struct StartMatchData {
    unsigned int matchID;
    MatchType type;
    int numOfPlayers;
    std::vector<ClientMatchInfo> players;
};

inline StartMatchData DeserializeMatch(const std::string& serialized) {
    StartMatchData data;
    std::istringstream ss(serialized);
    std::string token;
    std::vector<std::string> tokens;

    // Separar todos los tokens por ':'
    while (std::getline(ss, token, ':')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 3) throw std::runtime_error("Invalid serialized StartMatchData");

    // Parse matchID, matchType, numOfPlayers
    data.matchID = std::stoi(tokens[0]);
    data.type = (tokens[1] == "RANKED") ? MatchType::RANKED : MatchType::NORMAL;
    data.numOfPlayers = std::stoi(tokens[2]);
    
    return data;
}

inline StartMatchData DeserializePlayers(const std::string& serialized, StartMatchData data) 
{
    std::istringstream ss(serialized);
    std::string token;
    std::vector<std::string> tokens;

    while (std::getline(ss, token, ':')) { tokens.push_back(token); }

    if (tokens.size() < 3) throw std::runtime_error("Invalid serialized players");

    for (size_t i = 3; i + 2 < tokens.size(); i += 3) {
        ClientMatchInfo p;
        std::optional<sf::IpAddress> ipAdress = sf::IpAddress::resolve(tokens[i]);
        p.ip = ipAdress.value();
        p.port = static_cast<unsigned short>(std::stoi(tokens[i + 1]));
        p.playerID = std::stoi(tokens[i + 2]);
        data.players.push_back(p);
    }

    return data;
}
