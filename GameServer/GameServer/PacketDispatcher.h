#pragma once
#include "NetworkDefs.h"
#include <functional>
#include <thread>
#include <mutex>
#include <queue>

class PacketDispatcher
{
public:
    PacketDispatcher();
    ~PacketDispatcher();

    void EnqueuePacket(const RawPacketJob& job);
    void RegisterHandler(PacketType type, std::function<void(const RawPacketJob&)> handler);

    void Start();
    void Stop();

private:
    std::queue<RawPacketJob> _queueNormal;
    std::queue<RawPacketJob> _queueUrgent;
    std::queue<RawPacketJob> _queueCritical;
    std::map<PacketType, std::function<void(const RawPacketJob&)>> _handlers;
    std::mutex _mutex;
    std::atomic<bool> _running;
    std::thread _dispatchThread;

    void DispatchLoop();
};

