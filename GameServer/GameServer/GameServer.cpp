#include "GameServer.h"

sf::UdpSocket* gGameServerSocket = nullptr;

GameServer::GameServer() {}
GameServer::~GameServer() { _socket.unbind(); }
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
    // Extrae matchID desde el contenido (formato: matchID:payload)
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

    {
        std::lock_guard<std::mutex> lock(_matchMutex);

        if (_activeMatches.find(matchID) != _activeMatches.end()) {
            WriteConsole("[GAMESERVER] Match ID '", matchID, "' already in use.");
            SendDatagram(_socket, PacketHeader::CRITICAL, PacketType::MATCH_USED, "", job.sender.value(), job.port);
            return;
        }

        auto instance = std::make_shared<GameInstance>(matchData);
        _activeMatches[matchID] = instance;

        SendDatagram(_socket, PacketHeader::CRITICAL, PacketType::MATCH_UNIQUE, "", job.sender.value(), job.port);

        std::thread gameThread([instance]() {
            instance->Run();
            });
        gameThread.detach();

        WriteConsole("[GAMESERVER] Created match ", matchID, " with ", matchData.players.size(), " players.");
    }
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
                if (job.type == PacketType::MATCH_FOUND) 
                {
                    _dispatcher.EnqueuePacket(job);
                }
                else if (job.type == PacketType::CREATE_MATCH)
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
