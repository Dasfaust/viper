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

struct P2ClientTelemetry : Packet
{
	float mouseX;
	float mouseY;
	float scrollX;
	float scrollY;

	make_serializable(CEREAL_NVP(mouseX), CEREAL_NVP(mouseY), CEREAL_NVP(scrollX), CEREAL_NVP(scrollY));
};

struct P3ServerTelemetry : Packet
{
	float ping;
	float serverDelta;
	float serverTick;
	float serverIpDelta;
	float serverIpTick;
	float serverIpIncoming;
	float serverIpOutgoing;
	float serverNsTick;
	float worldDelta;
	float worldTick;
	int worldTps;

	make_serializable(CEREAL_NVP(ping), CEREAL_NVP(serverDelta), CEREAL_NVP(serverTick), CEREAL_NVP(serverIpDelta), CEREAL_NVP(serverIpTick), CEREAL_NVP(serverIpIncoming), CEREAL_NVP(serverIpOutgoing), CEREAL_NVP(serverNsTick), CEREAL_NVP(worldDelta), CEREAL_NVP(worldTick), CEREAL_NVP(worldTps));
};