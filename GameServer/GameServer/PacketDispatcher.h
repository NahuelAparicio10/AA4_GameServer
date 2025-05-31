#pragma once
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <string>
#include <vector>
#include <SFML/Network.hpp>
#include "PacketParser.h"
#include "Constants.h"

struct QueuedPacket {
    std::vector<char> data;
    std::size_t size;
   std::string senderIp;
    unsigned short senderPort;
};

class PacketDispatcher {
public:
    PacketDispatcher();
    ~PacketDispatcher();

    void Dispatch(PacketHeader headerType, const char* data, std::size_t size, const std::optional<sf::IpAddress>& senderIp, unsigned short senderPort);

private:
    std::queue<QueuedPacket> queue;
    std::mutex queueMutex;
    std::condition_variable queueCv;
    std::atomic<bool> running;
    std::thread dispatchThread;

    void DispatchLoop();

    void HandlePacketInQueue(const QueuedPacket& packet);
    void HandleUrgent(const char* data, std::size_t size, const std::optional<sf::IpAddress>& senderIpStr, unsigned short senderPort);
};
