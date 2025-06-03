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
                            SendDatagram(GameServerSocket(), PacketHeader::CRITIC, PacketType::ACK_JOINED, _data.numOfPlayers + "", p.ip, p.port);
                            break;
                        }
                    }
                }
            }


            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    WriteConsole("[MATCH ", _data.matchID, "] All players joined. Starting game logic...");

    // -- Create players in Server

    for (const auto& p : _data.players)
    {
        _scene->AddPlayer(p.playerID);
    }

    bool playersCreated = false;

    CreatePlayersForMatch(playersCreated);

    WriteConsole("ALL PLAYERS CREATED");

    //- Main Loop of the match

    while (_running)
    {
        float dt = clock.restart().asSeconds();
        accumulator += dt;

        // - Process inconming packets outside of the fixed loop
        {
            std::lock_guard<std::mutex> lock(_queueMutex);
            while (!_packetQueue.empty())
            {
                RawPacketJob job = _packetQueue.front(); _packetQueue.pop();
                if (job.type == PacketType::PLAYER_MOVEMENT)
                {
                    HandlePlayerMovement(job);
                }
            }
        }

        //Simular lógica a pasos fijos
       //while (accumulator >= 0.033)
       //{
        _scene->Update(0.033);
        //accumulator -= 0.033;
    //}

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
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

//-- Handles all the creation of players systems, sends starting postion of players to clients and waits for them to create
//-- the players, when its done continues the loop to main loop
void GameInstance::CreatePlayersForMatch(bool& playersCreated)
{
    while (!playersCreated)
    {
        std::unordered_set<unsigned int> ackedPlayers;

        int maxRetries = 5;
        int currentRetry = 0;

        while (currentRetry < maxRetries && ackedPlayers.size() < _data.players.size())
        {
            // - Sends to every client playerID:playerposX:playerposY|next players
            for (const auto& receiver : _data.players)
            {
                std::string msg;
                for (const auto& sender : _data.players)
                {
                    sf::Vector2f playerPos = _scene->GetPlayerPositionByID(sender.playerID);
                    msg += std::to_string(sender.playerID) + ":" +
                        std::to_string(playerPos.x) + ":" +
                        std::to_string(playerPos.y) + "|";
                }
                msg.pop_back();

                SendDatagram(GameServerSocket(), PacketHeader::CRITIC, PacketType::CREATE_PLAYER, msg, receiver.ip, receiver.port);
            }

            // - Waits for ACK_PLAYERS_CREATED from both clients during 500 ms if ack is no recieved tries create player again
            auto start = std::chrono::steady_clock::now();
            while (std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
            {
                std::lock_guard<std::mutex> lock(_queueMutex);
                while (!_packetQueue.empty())
                {
                    RawPacketJob job = _packetQueue.front(); _packetQueue.pop();

                    if (job.type == PacketType::ACK_PLAYERS_CREATED)
                    {
                        // - If job type is ACK_CREATE marks which player sent it
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

            if (ackedPlayers.size() == _data.players.size())
            {
                playersCreated = true;
            }
        }
    }
}





// -- Given an ID sends the packet to that client
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

//-- Given an ID sends the packet to every client unless ID client.

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

