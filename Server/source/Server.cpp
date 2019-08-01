#include "Server.hpp"

void Telemetry::onTick()
{
	auto server = getParent<Server>();
	for (auto&& kv : server->clients)
	{
		P0Telemetry tel;
		tel.serverStatus = 1;
		server->p0Handler->enqueue(tel, { kv.first });
	}
}

void KeepAlive::onTick()
{
	auto server = getParent<Server>();
	for (auto&& kv : server->clients)
	{
		if (timesince(kv.second.lastSeen) > 2000.0)
		{
			server->onDisconnect(kv.first);
		}
	}
};


void Server::onStart()
{
	wo = initModule<World>("world");
	ns = initModule<NetServer>("net");

	p0Handler = ns->registerPacket<P0Telemetry>(0);
	p0Listener = p0Handler->listen(0, [](P0Telemetry& packet, std::vector<std::shared_ptr<Module>> mods)
	{
		auto server = std::dynamic_pointer_cast<Server>(mods[0]);

		if (!server->clients.count(packet.client))
		{
			server->clients[packet.client] = { packet.client, tnowns() };
			info("Client has connected: %s", boost::lexical_cast<std::string>(packet.client).c_str());
		}
		if (packet.clientStatus == 0)
		{
			server->onDisconnect(packet.client);
		}
		else
		{
			server->clients[packet.client].lastSeen = tnowns();
		}
	}, { getParent<Viper>()->getModule("server") });

	auto tl = initModule<Telemetry>("telemetry", 500.0);
	auto ka = initModule<KeepAlive>("keepalive", 1000.0);

	for (auto&& kv : modules)
	{
		kv.second->onStart();
	}
};

void Server::onDisconnect(uid client)
{
	clients.erase(client);
	info("Client has disconnected: %s", boost::lexical_cast<std::string>(client).c_str());
};


void Server::onTick()
{
	p0Listener->poll();

	tickModules();
};

void Server::onShutdown()
{
	for (auto&& kv : modules)
	{
		kv.second->onShutdown();
	}
};