#pragma once
#include <SFML/Network.hpp>
#include <vector>
#include <optional>
#include "StartMatchData.h"
#include "NetworkDefs.h"
#include <mutex>
#include <queue>
#include "ConsoleUtils.h"

// -- Represents one match, its start in a exclusive port

class GameInstance
{
public:
    GameInstance(const StartMatchData& data);
    void EnqueuePacket(const RawPacketJob& job);
    void Run();
    bool IsRunning() const { return _running; }
private:
    StartMatchData _data;
    std::vector<ClientMatchInfo> _connectedPlayers;

    std::queue<RawPacketJob> _packetQueue;
    std::mutex _queueMutex;
    std::atomic<bool> _running = true;
};

