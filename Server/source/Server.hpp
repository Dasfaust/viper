#pragma once
#include "interface/Modular.hpp"
#include "world/World.hpp"
#include "net/NetServer.hpp"
#include "net/Packets.hpp"

struct NetworkClient
{
	uid id;
	time_val lastSeen;
};

class Telemetry : public Module
{
public:
	void onTick() override;
};

class KeepAlive : public Module
{
	void onTick() override;
};

class Server : public Module, public Modular
{
public:
	std::shared_ptr<World> wo;
	std::shared_ptr<NetServer> ns;
	flatmap(uid, NetworkClient) clients;
	std::shared_ptr<PacketHandler<P0Telemetry>> p0Handler;
	std::shared_ptr<Listener<P0Telemetry>> p0Listener;

	void onStart() override;

	void onDisconnect(uid client);

	void onTick() override;

	void onShutdown() override;
};
