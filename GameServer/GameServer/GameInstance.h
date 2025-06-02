#pragma once
#include <SFML/Network.hpp>
#include <vector>
#include <optional>
#include "StartMatchData.h"
#include "NetworkDefs.h"
#include <mutex>
#include <queue>
#include "ConsoleUtils.h"
#include "GameScene.h"
#include <unordered_set>

// -- Represents one match, its start in a exclusive port

class GameInstance
{
public:
    GameInstance(const StartMatchData& data);
    void EnqueuePacket(const RawPacketJob& job);
    void Run();
    bool IsRunning() const { return _running; }
private:
    void HandlePlayerMovement(const RawPacketJob& job);
    void SendToPlayer(unsigned int playerID, PacketHeader header, PacketType type, const std::string& content);
    void BroadcastToOthers(unsigned int senderID, PacketHeader header, PacketType type, const std::string& content);

    StartMatchData _data;
    std::vector<ClientMatchInfo> _connectedPlayers;

    std::queue<RawPacketJob> _packetQueue;
    std::mutex _queueMutex;
    std::atomic<bool> _running = true;

    std::map<unsigned int, MovementPacket> _lastClientReported;
    GameScene* _scene = nullptr;
};

