#pragma once
#include "../interface/PacketFactory.hpp"
#include "../interface/UDP.hpp"

struct P0Telemetry : Packet
{
	uint32 serverStatus = 0;
	uint32 clientStatus = 0;

	make_serializable(CEREAL_NVP(serverStatus), CEREAL_NVP(clientStatus));
};