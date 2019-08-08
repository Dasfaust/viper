#pragma once
#include "interface/Modular.hpp"
#include "net/NetClient.hpp"
#include "surface/WindowManager.hpp"
#include "input/InputManager.hpp"

class Client : public Module, public Modular
{
public:
	std::atomic_bool isConnected = false;

	std::shared_ptr<Listener<ClientConnectedEvent>> clientConnected;
	std::shared_ptr<Listener<ClientDisconnectedEvent>> clientDisconnected;
	std::shared_ptr<PacketHandler<P1Nickname>> p1Handler;
	std::shared_ptr<Listener<P1Nickname>> p1Listener;
	std::shared_ptr<PacketHandler<P2ClientTelemetry>> p2Handler;

	std::shared_ptr<InputManager> in;

	void onStart() override;

	void onTick() override;

	void onShutdown() override;
};

class Telemetry : public Module
{
public:
	std::shared_ptr<Client> cl;
	std::shared_ptr<WindowManager> wm;

	void onStart() override;
	void onTick() override;
};