#include "Client.hpp"

void Telemetry::onStart()
{
	cl = getParent<Client>();
	wm = cl->getModule<WindowManager>("wm");
};

void Telemetry::onTick()
{
	if (cl->isConnected.load())
	{
		P2ClientTelemetry packet;
		packet.mouseX = (float)wm->mouseX;
		packet.mouseY = (float)wm->mouseY;
		packet.scrollX = (float)wm->scrollX;
		packet.scrollY = (float)wm->scrollY;
		cl->p2Handler->enqueue(UDP, packet);
	}
};


void Client::onStart()
{
	auto nc = initModule<NetClient>("net");
	auto wm = initModule<WindowManager>("wm");

	for (auto&& kv : modules)
	{
		kv.second->onStart();
	}

	p1Handler = nc->registerPacket<P1Nickname>(1);
	p1Listener = p1Handler->listen(0, [](P1Nickname& packet, std::vector<std::shared_ptr<Module>> mods)
	{
		info("Client nickname is %s", packet.name.c_str());
	}, { });

	auto ccHandler = getParent<Modular>()->getModule<Events>("events")->getModule<EventHandler<ClientConnectedEvent>>("client_clientconnectedevent");
	clientConnected = ccHandler->listen(0, [](ClientConnectedEvent& ev, std::vector<std::shared_ptr<Module>> mods)
	{
		info("Connected to server");
		auto cl = std::dynamic_pointer_cast<Client>(mods[0]);
		set_atom(cl->isConnected, true, bool);
		P1Nickname nick;
		nick.name = "dasfaust";
		cl->p1Handler->enqueue(TCP, nick, { });
	}, { getParent<Modular>()->getModule("client") });

	auto cdHandler = getParent<Modular>()->getModule<Events>("events")->getModule<EventHandler<ClientDisconnectedEvent>>("client_clientdisconnectedevent");
	clientDisconnected = cdHandler->listen(0, [](ClientDisconnectedEvent& ev, std::vector<std::shared_ptr<Module>> mods)
	{
		warn("Disconnected from server: %s", ev.reason.c_str());
		auto cl = std::dynamic_pointer_cast<Client>(mods[0]);
		set_atom(cl->isConnected, false, bool);
	}, { getParent<Modular>()->getModule("client") });

	p2Handler = nc->registerPacket<P2ClientTelemetry>(2);
	auto tel = initModule<Telemetry>("telemetry", 100.0);
	tel->onStart();
};

void Client::onTick()
{
	clientConnected->poll();
	clientDisconnected->poll();
	p1Listener->poll();
	tickModules();
};

void Client::onShutdown()
{
	for (auto&& kv : modules)
	{
		kv.second->onShutdown();
	}
};