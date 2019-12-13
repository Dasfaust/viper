#pragma once
#include "interface/Modular.hpp"
#include "world/World.hpp"
#include "net/NetServer.hpp"
#include "net/Packets.hpp"

struct NetworkClient
{
	uid id;
	InetAddress address;
	std::string nickname;
	float mouseX;
	float mouseY;
	float scrollX;
	float scrollY;
	Time::point lastUpdate;
	float ping;
};

class ServerTelemetry;

class Server : public Module, public Modular, public Threadable
{
public:
	bool async = true;
	std::shared_ptr<World> world;
	std::shared_ptr<NetServer> ns;
	std::shared_ptr<Listener<ClientConnectedEvent>> clientConnected;
	std::shared_ptr<Listener<ClientDisconnectedEvent>> clientDisconnected;
	umap(uid, NetworkClient) clients;
	std::shared_ptr<PacketHandler<P1Nickname>> p1Handler;
	std::shared_ptr<Listener<P1Nickname>> p1Listener;
	std::shared_ptr<PacketHandler<P2ClientTelemetry>> p2Handler;
	std::shared_ptr<Listener<P2ClientTelemetry>> p2Listener;
	std::shared_ptr<PacketHandler<P3ServerTelemetry>> p3Handler;
	std::shared_ptr<ServerTelemetry> tel;

	void onStart() override;

	void onTickAsync() override;

	void onTick() override
	{
		if (!async)
		{
			onTickAsync();
		}
	};

	void onShutdown() override;
};

class ServerTelemetry : public Module
{
public:
	std::shared_ptr<Server> server;

	void onStart() override;
	void onTick() override;
};