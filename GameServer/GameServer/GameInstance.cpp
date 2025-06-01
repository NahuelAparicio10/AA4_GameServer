#include "GameInstance.h"
#include <iostream>

GameInstance::GameInstance(const StartMatchData& data) : _data(data) {}

void GameInstance::EnqueuePacket(const RawPacketJob& job) 
{
    std::lock_guard<std::mutex> lock(_queueMutex);
    _packetQueue.push(job);
}

void GameInstance::Run() 
{
    WriteConsole("[MATCH ", _data.matchID , "] Started with " , _data.players.size() , " players." );

    // Esperar JOIN_GAME de todos
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
                            WriteConsole("[MATCH " , _data.matchID , "] Player joined: " , p.ip , ":" , p.port);
                            break;
                        }
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    WriteConsole("[MATCH ", _data.matchID, "] All players joined. Starting game logic...");
    // Aquí iría el bucle principal del juego
    while (_running) {
        {
            std::lock_guard<std::mutex> lock(_queueMutex);
            while (!_packetQueue.empty()) {
                RawPacketJob job = _packetQueue.front(); _packetQueue.pop();
                // Procesar acciones de jugadores...
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(33)); // ~30 FPS
    }
}
