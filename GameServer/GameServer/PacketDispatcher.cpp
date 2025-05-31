#include "PacketDispatcher.h"
#include "ConsoleUtils.h"

PacketDispatcher::PacketDispatcher() : running(true), dispatchThread(&PacketDispatcher::DispatchLoop, this) {}

PacketDispatcher::~PacketDispatcher() {
    running = false;
    queueCv.notify_all();
    if (dispatchThread.joinable()) dispatchThread.join();
}

void PacketDispatcher::Dispatch(PacketHeader headerType, const char* data, std::size_t size, const std::optional<sf::IpAddress>& senderIp, unsigned short senderPort) {
   
    if (headerType & URGENT) {
       
        std::thread urgentThread(&PacketDispatcher::HandleUrgent, this, data, size, senderIp, senderPort);
        urgentThread.detach();
        return;
    }

    QueuedPacket packet;
    packet.data = std::vector<char>(data, data + size);
    packet.size = size;
    //packet.senderIp = senderIp.value().toString();
    packet.senderPort = senderPort;

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        queue.push(packet);
    }
    queueCv.notify_one();
}

void PacketDispatcher::DispatchLoop() {
    while (running) {
        QueuedPacket packet;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCv.wait(lock, [&]() { return !queue.empty() || !running; });
            if (!running && queue.empty()) break;

            packet = queue.front();
            queue.pop();
        }

        HandlePacketInQueue(packet);
    }
}

void PacketDispatcher::HandlePacketInQueue(const QueuedPacket& packet) {
    try {
        PacketParser parser(packet.data.data(), packet.size);
        PacketHeader type = parser.ReadPacketType();

        int payload = parser.ReadInt();
        WriteConsole("[QUEUED] Payload: ", payload, "\n");

        if (type == CRITIC)
            WriteConsole("[QUEUED] CRITIC packet ejecutado en hilo común.\n");
        else
            WriteConsole("[QUEUED] NORMAL packet ejecutado en hilo común.\n");

    }
    catch (const std::exception& e) {
        WriteConsole("[QUEUED] Error al procesar: ", e.what(), "\n");
    }
}

void PacketDispatcher::HandleUrgent(const char* data, std::size_t size, const std::optional<sf::IpAddress>& senderIpStr, unsigned short senderPort) {
    try {
        PacketParser parser(data, size);
        PacketHeader type = parser.ReadPacketType();

        WriteConsole("[URGENT] Packet desde: ", senderIpStr.value(), "\n");
        int payload = parser.ReadInt();
        WriteConsole("[URGENT] Payload: ", payload, "\n");

        if (type & CRITIC)
            WriteConsole("[URGENT] (también CRITIC)\n");

    }
    catch (const std::exception& e) {
        WriteConsole("[URGENT] Error al procesar: ", e.what(), "\n");
    }
}
