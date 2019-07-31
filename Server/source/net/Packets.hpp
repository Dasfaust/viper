#pragma once
#include "../interface/PacketFactory.hpp"
#include "../interface/UDP.hpp"

struct P0Handshake : Packet
{
	std::string key;

	make_serializable(key);
};