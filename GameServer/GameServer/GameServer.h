#pragma once
#include <SFML/Network.hpp>
#include <atomic>
#include <thread>
#include <vector>
#include <map>
#include "PacketDispatcher.h"
#include "Constants.h"
#include "StartMatchData.h" 
#include "GameInstance.h"
#include "ConsoleUtils.h"

// -- Manages the global server listents for START_MATCH and throws threads with GameInstances

class GameServer
{
public:
    GameServer();
    ~GameServer();
    void DispatchPacket(const RawPacketJob& job);
    void HandleCreateMatch(const RawPacketJob& job);
    void Run(std::atomic<bool>& running);
private:
    bool InitializeSocket();

    sf::UdpSocket _socket;
    PacketDispatcher _dispatcher;

    unsigned int _nextMatchID = 0;
    std::map<unsigned int, std::shared_ptr<GameInstance>> _activeMatches;
    std::mutex _matchMutex;
};

