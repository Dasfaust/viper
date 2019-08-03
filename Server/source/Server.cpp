#include "Server.hpp"

void Server::onStart()
{
	wo = initModule<World>("world");
	ns = initModule<NetServer>("net");

	for (auto&& kv : modules)
	{
		kv.second->onStart();
	}

	p1Handler = ns->registerPacket<P1Nickname>(1);
	p1Listener = p1Handler->listen(0, [](P1Nickname& packet, std::vector<std::shared_ptr<Module>> mods)
	{
		auto sr = std::dynamic_pointer_cast<Server>(mods[0]);
		sr->clients[packet.client].nickname = packet.name;
		info("Changed nickname: %s (%s)", packet.name.c_str(), boost::lexical_cast<std::string>(packet.client).c_str());
		sr->p1Handler->enqueue(TCP, packet, { packet.client });
	}, { getParent<Modular>()->getModule("server") });

	auto ccHandler = getParent<Modular>()->getModule<Events>("events")->getModule<EventHandler<ClientConnectedEvent>>("server_clientconnectedevent");
	clientConnected = ccHandler->listen(0, [](ClientConnectedEvent& ev, std::vector<std::shared_ptr<Module>> mods)
	{
		debug("Client connected: %s", boost::lexical_cast<std::string>(ev.id).c_str());
		auto sr = std::dynamic_pointer_cast<Server>(mods[0]);
		sr->clients[ev.id] = { ev.id, ev.address };
	}, { getParent<Modular>()->getModule("server") });

	auto cdHandler = getParent<Modular>()->getModule<Events>("events")->getModule<EventHandler<ClientDisconnectedEvent>>("server_clientdisconnectedevent");
	clientDisconnected = cdHandler->listen(0, [](ClientDisconnectedEvent& ev, std::vector<std::shared_ptr<Module>> mods)
	{
		debug("%s disconnected: %s", boost::lexical_cast<std::string>(ev.id).c_str(), ev.reason.c_str());
		auto sr = std::dynamic_pointer_cast<Server>(mods[0]);
		sr->clients.erase(ev.id);
	}, { getParent<Modular>()->getModule("server") });
};

void Server::onTick()
{
	clientConnected->poll();
	clientDisconnected->poll();
	p1Listener->poll();
	tickModules();
};

void Server::onShutdown()
{
	for (auto&& kv : modules)
	{
		kv.second->onShutdown();
	}
};