#include "NetworkManager.h"

NetworkManager::NetworkManager()
{

}

bool NetworkManager::Init()
{
    if (_socket.bind(GameServerPort) != sf::Socket::Status::Done) {
        WriteConsole("[NETWORK_MANAGER] No se ha bindeado con el puerto: ", GameServerPort);
        return false;
    }

    _socket.setBlocking(false);
    WriteConsole("[NETWORK_MANAGER] GameServer escuchando en puerto UDP : ", GameServerPort, "\n");
    return true;

}

void NetworkManager::PollSockets()
{
    char buffer[BUFFER_SIZE];
    std::size_t receivedSize = 0;
    std::optional<sf::IpAddress> senderIp;
    unsigned short senderPort;

    while (true) {
        sf::Socket::Status status = _socket.receive(buffer, BUFFER_SIZE, receivedSize, senderIp, senderPort);

        if (status == sf::Socket::Status::Done) {
            WriteConsole("[NETWORK_MANAGER] Packet recibido desde la ip: ", senderIp.value(), "\n", senderPort, "\n");

            HandlePacket(buffer, receivedSize, senderIp, senderPort);
        }
    }


}

void NetworkManager::HandlePacket(const char* data, std::size_t size, const std::optional<sf::IpAddress>& senderIp, unsigned senderPort) {
    try {
        PacketParser parser(data, size);
        PacketHeader headerType = parser.ReadPacketType();


        //WriteConsole(senderIp);
        _dispatcher.Dispatch(headerType, data, size, senderIp, senderPort);

    }
    catch (const std::exception& e) {
        WriteConsole("[NETWORK_MANAGER] Error al parsear paquete: ", e.what(), "\n");
    }
}