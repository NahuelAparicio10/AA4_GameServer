#include "PacketDispatcher.h"

PacketDispatcher::PacketDispatcher() : _running(false)
{
}

PacketDispatcher::~PacketDispatcher()
{
	Stop();
}

void PacketDispatcher::EnqueuePacket(const RawPacketJob& job)
{
	std::lock_guard<std::mutex> lock(_mutex);
    if (job.headerMask & PacketHeader::URGENT) {
        _queueUrgent.push(job);
    }
    else if (job.headerMask & PacketHeader::CRITIC) {
        _queueCritic.push(job);
    }
    else {
        _queueNormal.push(job);
    }
}

void PacketDispatcher::RegisterHandler(PacketType type, std::function<void(const RawPacketJob&)> handler) {	_handlers[type] = handler; }

void PacketDispatcher::Start()
{
	_running = true;
	_dispatchThread = std::thread(&PacketDispatcher::DispatchLoop, this);
}

void PacketDispatcher::Stop()
{
	_running = false;
	if (_dispatchThread.joinable()) _dispatchThread.join();
}

void PacketDispatcher::DispatchLoop()
{
    while (_running) 
    {
        RawPacketJob job;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (!_queueUrgent.empty()) {
                job = _queueUrgent.front();
                _queueUrgent.pop();
            }
            else if (!_queueCritic.empty()) {
                job = _queueCritic.front();
                _queueCritic.pop();
            }
            else if (!_queueNormal.empty()) {
                job = _queueNormal.front();
                _queueNormal.pop();
            }
            else {
                continue;
            }
        }

        auto it = _handlers.find(job.type);
        if (it != _handlers.end()) {
            it->second(job);
        }
    }
}
