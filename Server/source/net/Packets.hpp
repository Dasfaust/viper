#pragma once
#include "../interface/PacketFactory.hpp"
#include "../interface/IPHandler.hpp"

struct Packet : Event
{
	uid client;
};

struct P0Handshake : Packet
{
	uint32 token;
	uint32 status;

	make_serializable(CEREAL_NVP(token), CEREAL_NVP(status));
};

struct P1Nickname : Packet
{
	std::string name;

	make_serializable(CEREAL_NVP(name));
};