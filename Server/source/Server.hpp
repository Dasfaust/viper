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
};

class Server : public Module, public Modular
{
public:
	std::shared_ptr<World> wo;
	std::shared_ptr<NetServer> ns;
	std::shared_ptr<Listener<ClientConnectedEvent>> clientConnected;
	std::shared_ptr<Listener<ClientDisconnectedEvent>> clientDisconnected;
	boost::container::flat_map<uid, NetworkClient> clients;
	std::shared_ptr<PacketHandler<P1Nickname>> p1Handler;
	std::shared_ptr<Listener<P1Nickname>> p1Listener;
	std::shared_ptr<PacketHandler<P2ClientTelemetry>> p2Handler;
	std::shared_ptr<Listener<P2ClientTelemetry>> p2Listener;

	void onStart() override;

	void onTick() override;

	void onShutdown() override;
};
