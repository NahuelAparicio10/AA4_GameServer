#include <SFML/Graphics.hpp>
#include "GameServer.h"

std::atomic<bool> running(true);

int main()
{
	GameServer server;

	server.Run(running);

	return 0;
}