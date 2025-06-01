#include "GameInstance.h"
#include <iostream>

GameInstance::GameInstance(const StartMatchData& data) : _data(data) 
{

    _scene = new GameScene(static_cast<int>(data.players.size()));

    // Asigna playerID reales a GameObjects
    for (const auto& p : data.players)
    {
        _scene->AddPlayer(p.playerID); 
    }
}

void GameInstance::EnqueuePacket(const RawPacketJob& job) 
{
    std::lock_guard<std::mutex> lock(_queueMutex);
    _packetQueue.push(job);
}

void GameInstance::Run() 
{
    WriteConsole("[MATCH ", _data.matchID, "] Started with ", _data.players.size(), " players.");

    // Esperar a que se unan todos
    unsigned int joined = 0;
    while (joined < _data.players.size()) {
        {
            std::lock_guard<std::mutex> lock(_queueMutex);
            while (!_packetQueue.empty()) {
                RawPacketJob job = _packetQueue.front(); _packetQueue.pop();
                if (job.type == PacketType::JOIN_GAME) {
                    for (const auto& p : _data.players) {
                        if (p.ip == job.sender && p.port == job.port) {
                            _connectedPlayers.push_back(p);
                            joined++;
                            WriteConsole("[MATCH ", _data.matchID, "] Player joined: ", p.ip, ":", p.port);
                            break;
                        }
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    WriteConsole("[MATCH ", _data.matchID, "] All players joined. Starting game logic...");

    // Bucle de juego
    while (_running) {
        {
            std::lock_guard<std::mutex> lock(_queueMutex);
            while (!_packetQueue.empty()) {
                RawPacketJob job = _packetQueue.front(); _packetQueue.pop();
                if (job.type == PacketType::PLAYER_MOVEMENT)
                    HandlePlayerMovement(job);
            }
        }

        // Simula física y colisiones
        _scene->Update(0.033f);

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
                SendToPlayer(playerID, PacketHeader::CRITICAL, PacketType::RECONCILE, corrected.Serialize());
            }
            else
            {
                BroadcastToOthers(playerID, PacketHeader::NORMAL, PacketType::PLAYER_MOVEMENT, corrected.Serialize());
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(33)); // ~30 FPS
    }
}

void GameInstance::HandlePlayerMovement(const RawPacketJob& job)
{
    MovementPacket packet = MovementPacket::Deserialize(job.content);
    _lastClientReported[packet.playerID] = packet;

    GameObject* player = _scene->GetPlayerByID(packet.playerID);
    if (!player) return;

    Rigidbody2D* rb = player->GetComponent<Rigidbody2D>();
    if (!rb) return;

    // Simula el movimiento recibido
    rb->velocity = packet.velocity;
    player->transform->position = packet.position;
}
extern sf::UdpSocket* gGameServerSocket;

sf::UdpSocket& GameServerSocket() {
    return *gGameServerSocket;
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

