#pragma once
#include "interface/Modular.hpp"
#include "net/NetClient.hpp"

class Client : public Module, public Modular
{
public:
	std::shared_ptr<Listener<ClientConnectedEvent>> clientConnected;
	std::shared_ptr<Listener<ClientDisconnectedEvent>> clientDisconnected;
	std::shared_ptr<PacketHandler<P1Nickname>> p1Handler;
	std::shared_ptr<Listener<P1Nickname>> p1Listener;

	void onStart() override;

	void onTick() override;

	void onShutdown() override;
};
