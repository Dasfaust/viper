#pragma once
#include "../interface/PacketFactory.hpp"
#include "../interface/IPHandler.hpp"

struct Packet : Event
{
	uid client;
};

struct P0Telemetry : Packet
{
	uint32 serverStatus = 0;
	uint32 clientStatus = 0;

	make_serializable(CEREAL_NVP(serverStatus), CEREAL_NVP(clientStatus));
};