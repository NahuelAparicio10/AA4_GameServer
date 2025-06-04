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
    ~GameInstance();
    void EnqueuePacket(const RawPacketJob& job);
    void Run();
    void CheckDisconnections();
    void HandlePlayerDisconnected(unsigned int playerID);
    void HandleShootBullet(const RawPacketJob& job);
    void CreatePlayersForMatch(bool& playersCreated);
    bool IsRunning() const { return _running; }
private:
    void HandlePlayerMovement(const RawPacketJob& job);
    void SendToPlayer(unsigned int playerID, PacketHeader header, PacketType type, const std::string& content);
    void BroadcastToAll(PacketHeader header, PacketType type, const std::string& content);
    void BroadcastToOthers(unsigned int senderID, PacketHeader header, PacketType type, const std::string& content);

    StartMatchData _data;
    std::vector<ClientMatchInfo> _connectedPlayers;

    std::mutex _queueMutex;
    std::queue<RawPacketJob> _packetQueue;
    
    std::atomic<bool> _running = true;

    GameScene* _scene = nullptr;
    sf::Clock clock;
    float accumulator = 0;
    unsigned int _nextBulletID = 1;
    
    std::unordered_map<unsigned int, std::chrono::steady_clock::time_point> _lastRespond;
};

