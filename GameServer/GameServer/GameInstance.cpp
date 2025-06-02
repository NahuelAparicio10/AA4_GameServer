#include "GameInstance.h"
#include <iostream>

extern sf::UdpSocket* gGameServerSocket;

sf::UdpSocket& GameServerSocket() {
    return *gGameServerSocket;
}
GameInstance::GameInstance(const StartMatchData& data) : _data(data) 
{
    _scene = new GameScene(static_cast<int>(data.players.size()));

}

void GameInstance::EnqueuePacket(const RawPacketJob& job) 
{
    std::lock_guard<std::mutex> lock(_queueMutex);
    _packetQueue.push(job);
}

void GameInstance::Run()
{
    WriteConsole("[MATCH ", _data.matchID, "] Started with ", _data.players.size(), " players.");

    unsigned int joined = 0;
    while (joined < _data.players.size()) {
        {
            std::lock_guard<std::mutex> lock(_queueMutex);
            if (!_packetQueue.empty()) 
            {
                RawPacketJob job = _packetQueue.front(); _packetQueue.pop();
                if (job.type == PacketType::JOIN_GAME)
                {
                    for (const auto& p : _data.players)
                    {
                        if (p.ip == job.sender && p.port == job.port)
                        {
                            // - Check if player is already joined
                            bool alreadyJoined = false;
                            for (const auto& existing : _connectedPlayers)
                            {
                                if (existing.playerID == p.playerID)
                                {
                                    alreadyJoined = true;
                                    break;
                                }
                            }

                            if (alreadyJoined)
                                break; // - Ignores JOIN_GAME duplicateds

                            _connectedPlayers.push_back(p);
                            joined++;

                            WriteConsole("[MATCH ", _data.matchID, "] Player joined: ", p.ip, ":", p.port);

                            // --- ENVÍA CONFIRMACIÓN ---
                            SendDatagram(GameServerSocket(), PacketHeader::CRITIC, PacketType::ACK_JOINED, std::to_string(_data.matchID) + ":" + std::to_string(p.playerID), p.ip, p.port);
                            break;
                        }
                    }
                }
            }
           
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    WriteConsole("[MATCH ", _data.matchID, "] All players joined. Starting game logic...");

    bool playersCreated = false;
    std::unordered_map<unsigned int, int> ackRetries;


    // - Tries to send create players to clients
    while (!playersCreated)
    {
        std::unordered_set<unsigned int> ackedPlayers;

        int maxRetries = 5;
        int currentRetry = 0;

        while (currentRetry < maxRetries && ackedPlayers.size() < _data.players.size())
        {
            for (const auto& p : _data.players)
            {
                if (ackedPlayers.find(p.playerID) != ackedPlayers.end()) continue;

                sf::Vector2f playerPos = _scene->AddPlayer(p.playerID);
                std::string msg = std::to_string(p.playerID) + ":" + std::to_string(playerPos.x) + ":" + std::to_string(playerPos.y);

                SendDatagram(GameServerSocket(), PacketHeader::CRITIC, PacketType::CREATE_PLAYER, msg, p.ip, p.port);
            }

            // Espera ACKs durante 500 ms
            auto start = std::chrono::steady_clock::now();
            while (std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
            {
                std::lock_guard<std::mutex> lock(_queueMutex);
                while (!_packetQueue.empty())
                {
                    RawPacketJob job = _packetQueue.front(); _packetQueue.pop();

                    if (job.type == PacketType::ACK_PLAYERS_CREATED)
                    {
                        for (const auto& p : _data.players)
                        {
                            if (p.ip == job.sender && p.port == job.port)
                            {
                                if (ackedPlayers.find(p.playerID) == ackedPlayers.end())
                                {
                                    ackedPlayers.insert(p.playerID);
                                    WriteConsole("[MATCH ", _data.matchID, "] Received ACK_PLAYERS_CREATED from Player ", p.playerID);
                                }
                                break;
                            }
                        }
                    }
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            currentRetry++;
        }
    }

    // - Game Loop
    while (_running) {
        {
            std::lock_guard<std::mutex> lock(_queueMutex);
            while (!_packetQueue.empty()) {
                RawPacketJob job = _packetQueue.front(); _packetQueue.pop();
                if (job.type == PacketType::PLAYER_MOVEMENT)
                    HandlePlayerMovement(job);
            }
        }

        // Valida posición real vs lo enviado por cliente
        for (auto& [playerID, go] : _scene->GetPlayerMap())
        {
            if (_lastClientReported.count(playerID) == 0) continue;

            const MovementPacket& last = _lastClientReported[playerID];
            sf::Vector2f simulatedPos = go->transform->position;

            float dx = std::abs(simulatedPos.x - last.position.x);
            float dy = std::abs(simulatedPos.y - last.position.y);

            MovementPacket corrected = last;
            corrected.position = simulatedPos;

            if (dx > 10.f || dy > 10.f)
            {
                SendToPlayer(playerID, PacketHeader::URGENT, PacketType::RECONCILE, corrected.Serialize());
            }
            else
            {
                BroadcastToOthers(playerID, PacketHeader::NORMAL, PacketType::PLAYER_MOVEMENT, corrected.Serialize());
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(33)); // ~30 FPS
    }

}

    void GameInstance::HandlePlayerMovement(const RawPacketJob & job)
    {
        MovementPacket packet = MovementPacket::Deserialize(job.content);
        _lastClientReported[packet.playerID] = packet;

        GameObject* player = _scene->GetPlayerByID(packet.playerID);
        if (!player) return;

        Rigidbody2D* rb = player->GetComponent<Rigidbody2D>();
        if (!rb) return;

        rb->velocity = packet.velocity;
        player->transform->position = packet.position;

        // - Simula aquí físicas si quieres mayor fidelidad
        _scene->Update(0.033f); // Aproximadamente 30 FPS

        // -- Corrección de posición (reconciliación)
        MovementPacket correction;
        correction.matchID = packet.matchID;
        correction.playerID = packet.playerID;
        correction.tick = packet.tick;
        correction.position = player->transform->position;
        correction.velocity = rb->velocity;

        SendToPlayer(packet.playerID, PacketHeader::CRITIC, PacketType::RECONCILE, correction.Serialize());

        // -- Difusión a los demás para interpolación
        BroadcastToOthers(packet.playerID, PacketHeader::NORMAL, PacketType::PLAYER_MOVEMENT, correction.Serialize());
    }



void GameInstance::SendToPlayer(unsigned int playerID, PacketHeader header, PacketType type, const std::string& content)
{
    for (const auto& p : _connectedPlayers)
    {
        if (p.playerID == playerID)
        {
            SendDatagram(GameServerSocket(), header, type, content, p.ip, p.port);
            return;
        }
    }
}

void GameInstance::BroadcastToOthers(unsigned int senderID, PacketHeader header, PacketType type, const std::string& content)
{
    for (const auto& p : _connectedPlayers)
    {
        if (p.playerID != senderID)
        {
            SendDatagram(GameServerSocket(), header, type, content, p.ip, p.port);
        }
    }
}

