#include "NetworkManager.h"

NetworkManager::NetworkManager()
{

}

bool NetworkManager::Init()
{
	if (_socket.bind(GameServerPort) != sf::Socket::Status::Done) {
		WriteConsole("[NETWORK_MANAGER] No se ha bindeado con el puerto: ",GameServerPort);
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
            WriteConsole("[NETWORK_MANAGER] ", receivedSize," bytes de ", senderIp.value(),":", senderPort, "\n");
            

            HandlePacket(buffer, receivedSize, senderIp.value(), senderPort);
        }
        else {
            WriteConsole("[NETWORK_MANAGER] Error al recibir datos UDP: ", static_cast<int>(status),"\n");

            break;
        }
    }


}

void NetworkManager::HandlePacket(const char* data, std::size_t size, const sf::IpAddress& senderIp, unsigned senderPort)
{
    if (size == 0) return;
    uint8_t messageType = static_cast<uint8_t>(data[0]); 
    WriteConsole("[NETWORK_MANAGER] Tipo de mensaje recibido: ", messageType, "\n");
}
