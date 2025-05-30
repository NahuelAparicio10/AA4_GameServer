#pragma once
#include <iostream>
#include "Constants.h"
#include "ConsoleUtils.h"
#include <SFML/Network.hpp>
#include "PacketParser.h"

class NetworkManager
{
public:
	NetworkManager();

	bool Init();
	void PollSockets();



private:
	sf::UdpSocket _socket;
	static constexpr std::size_t BUFFER_SIZE = 1024;

	void HandlePacket(const char* data, std::size_t size, const sf::IpAddress& senderIp, unsigned senderPort);


};

