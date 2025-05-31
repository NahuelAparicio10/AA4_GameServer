#pragma once
#include <cstdint>
#include "Constants.h"

struct Packet {
	
	PacketHeader header;
	PacketType type;
	int id;
};

struct PacketStartMatch : Packet {
	int numPlayers;
	
};

struct PacketPlayerReady : Packet {
	int playerID;

};

struct PacketShoot : Packet {



};

struct PacketInput : Packet {



};

struct PacketMovement : Packet {



};