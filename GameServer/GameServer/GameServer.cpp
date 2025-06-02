#include "GameServer.h"

sf::UdpSocket* gGameServerSocket = nullptr;

GameServer::GameServer() {}
GameServer::~GameServer() { _socket.unbind(); }

// -- Bind to the port
bool GameServer::InitializeSocket() {
    if (_socket.bind(GameServerPort) != sf::Socket::Status::Done) {
        WriteConsole("[GAMESERVER] Failed to bind UDP port.");
        return false;
    }
    gGameServerSocket = &_socket;
    WriteConsole("[GAMESERVER] Listening on port ", GameServerPort);
    return true;
}

void GameServer::DispatchPacket(const RawPacketJob& job) 
{
    std::istringstream ss(job.content);
    std::string matchIDStr;
    std::getline(ss, matchIDStr, ':');
    unsigned int matchID = std::stoi(matchIDStr);

    std::lock_guard<std::mutex> lock(_matchMutex);
    auto it = _activeMatches.find(matchID);
    if (it != _activeMatches.end()) {
        it->second->EnqueuePacket(job);
    }
    else {
        WriteConsole("[GameServer] Match ID no encontrado:", matchID);
    }
}

void GameServer::HandleCreateMatch(const RawPacketJob& job)
{
    StartMatchData matchData = DeserializeMatch(job.content);
    unsigned int matchID = matchData.matchID;

    std::lock_guard<std::mutex> lock(_matchMutex);

    // - If the match ID is already in use returns to SS MATCH_USED and wait for another ID to come

    if (_activeMatches.find(matchID) != _activeMatches.end()) {
        WriteConsole("[GAMESERVER] Match ID '", matchID, "' already in use.");
        SendDatagram(_socket, PacketHeader::URGENT, PacketType::MATCH_USED, "", job.sender.value(), job.port);
        return;
    }

    matchData = DeserializePlayers(job.content, matchData);

    for (int i = 0; i < matchData.players.size(); i++)
    {
        std::cout << matchData.players[i].playerID << std::endl;
    }

    // Creates and registers a new GameInstance
    auto match = std::make_shared<GameInstance>(matchData);
    _activeMatches[matchID] = match;
    std::thread([match]() { match->Run(); }).detach();

    // Confirmar creación única
    SendDatagram(_socket, PacketHeader::URGENT, PacketType::MATCH_UNIQUE, "", job.sender.value(), job.port);
    WriteConsole("[GAMESERVER] Created match ", matchID, " with ", matchData.players.size(), " players.");
}

void GameServer::Run(std::atomic<bool>& running) {
    if (!InitializeSocket()) return;

    _dispatcher.RegisterHandler(PacketType::CREATE_MATCH, [this](const RawPacketJob& job) {
        HandleCreateMatch(job);
        });

    _dispatcher.Start();

    while (running) {
        char buffer[1024];
        std::size_t received;
        std::optional<sf::IpAddress> sender = std::nullopt;
        unsigned short port;

        if (_socket.receive(buffer, sizeof(buffer), received, sender, port) == sf::Socket::Status::Done && sender.has_value()) {
            RawPacketJob job;
            if (ParseRawDatagram(buffer, received, job, sender.value(), port)) {
                if (job.type == PacketType::CREATE_MATCH)
                {
                    _dispatcher.EnqueuePacket(job);
                }
                else 
                {
                    DispatchPacket(job);
                }
            }
        }
    }

    _dispatcher.Stop();
}
