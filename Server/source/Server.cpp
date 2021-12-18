#include "Server.hpp"

void ServerTelemetry::onStart()
{
	server = getParent<Server>();
};

void ServerTelemetry::onTick()
{
	for (auto&& kv : server->clients)
	{
		P3ServerTelemetry packet;
		packet.ping = kv.second.ping;
		packet.serverDelta = server->deltaTimeMs;
		packet.serverTick = server->tickTimeMs;
		packet.serverIpDelta = Time::toMilliseconds(server->ns->ip->deltaTime);
		packet.serverIpTick = Time::toMilliseconds(server->ns->ip->tickTime);
		packet.serverIpIncoming = Time::toMilliseconds(server->ns->ip->incomingTime);
		packet.serverIpOutgoing = Time::toMilliseconds(server->ns->ip->outgoingTime);
		packet.serverNsTick = server->ns->tickTimeMs;
		packet.worldDelta = server->world->deltaTimeMs;
		packet.worldTick = server->world->tickTimeMs;
		// todo
		packet.worldTps = 0;
		server->p3Handler->enqueue(UDP, packet, { kv.first });
	}
};

void Server::onStart()
{
	ns = initModule<NetServer>("net");
	world = initModule<World>("world", (1.0f / 30.0f) * 1000.0f);
	tel = initModule<ServerTelemetry>("telemetry", 100.0f);
	
	for (auto&& kv : modules)
	{
		kv.second->onStart();
	}

	p1Handler = ns->registerPacket<P1Nickname>(1);
	p1Listener = p1Handler->listen(0, [](P1Nickname& packet, std::vector<std::shared_ptr<Module>> mods)
	{
		auto sr = std::dynamic_pointer_cast<Server>(mods[0]);
		sr->clients[packet.client].nickname = packet.name;
		info("Changed nickname: %s (%s)", packet.name.c_str(), packet.client.c_str());
		sr->p1Handler->enqueue(TCP, packet, { packet.client });
	}, { getParent<Modular>()->getModule("server") });

	auto ccHandler = getParent<Modular>()->getModule<Events>("events")->getModule<EventHandler<ClientConnectedEvent>>("server_clientconnectedevent");
	clientConnected = ccHandler->listen(0, [](ClientConnectedEvent& ev, std::vector<std::shared_ptr<Module>> mods)
	{
		info("Client connected: %s", ev.id.c_str());
		auto sr = std::dynamic_pointer_cast<Server>(mods[0]);
		sr->clients[ev.id] = { ev.id, ev.address };
	}, { getParent<Modular>()->getModule("server") });

	auto cdHandler = getParent<Modular>()->getModule<Events>("events")->getModule<EventHandler<ClientDisconnectedEvent>>("server_clientdisconnectedevent");
	clientDisconnected = cdHandler->listen(0, [](ClientDisconnectedEvent& ev, std::vector<std::shared_ptr<Module>> mods)
	{
		info("%s disconnected: %s", ev.id.c_str(), ev.reason.c_str());
		auto sr = std::dynamic_pointer_cast<Server>(mods[0]);
		sr->clients.erase(ev.id);
	}, { getParent<Modular>()->getModule("server") });

	p2Handler = ns->registerPacket<P2ClientTelemetry>(2);
	p2Listener = p2Handler->listen(0, [](P2ClientTelemetry& packet, std::vector<std::shared_ptr<Module>> mods)
	{
		auto sr = std::dynamic_pointer_cast<Server>(mods[0]);
		sr->clients[packet.client].mouseX = packet.mouseX;
		sr->clients[packet.client].mouseY = packet.mouseY;
		sr->clients[packet.client].scrollX = packet.scrollX;
		sr->clients[packet.client].scrollY = packet.scrollY;
		auto now = Time::now();
		sr->clients[packet.client].ping = Time::toMilliseconds(Time::since(now, sr->clients[packet.client].lastUpdate)) - 100.0f;
		sr->clients[packet.client].lastUpdate = now;
	}, { getParent<Modular>()->getModule("server") });

	p3Handler = ns->registerPacket<P3ServerTelemetry>(3);
};

void Server::onTick()
{
	if (firstTick)
	{
		if (async)
		{
			getParent<Modular>()->getModule<Threads>("threads")->watch(std::dynamic_pointer_cast<Threadable>(shared_from_this()));
			sleep = false;
			start();
		}

		firstTick = false;
	}

	if (!async)
	{
		onTickAsync();
	}
};

void Server::onTickAsync()
{		
	clientConnected->poll();
	clientDisconnected->poll();
	p1Listener->poll();
	p2Listener->poll();

	tickModules();
};

void Server::onShutdown()
{
	for (auto&& kv : modules)
	{
		kv.second->onShutdown();
	}
};