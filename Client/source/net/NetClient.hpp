#pragma once
#include "net/Packets.hpp"
#include "thread/Threads.hpp"

#ifdef VIPER_WIN64
#include "UDPClientWin.hpp"
#endif

class NetClient : public Module, public PacketFactory
{
public:
	std::shared_ptr<UDP> udp;
	std::shared_ptr<PacketHandler<P0Handshake>> p0Handler;
	std::shared_ptr<Listener<P0Handshake>> p0Listener;

	void onStart() override
	{
		p0Handler = registerPacket<P0Handshake>(0);
		p0Listener = p0Handler->listen(0, [](P0Handshake& packet, std::vector<std::shared_ptr<Module>> mods)
		{
			debug(packet.key);
		}, { });

#ifdef VIPER_WIN64
		udp = std::make_shared<UDPClientWin>();
#endif
		udp->address = { "127.0.0.1", 481516 };
		getParent<Module>()->getParent<Modular>()->getModule<Threads>("threads")->watch(udp);
		udp->start();

		P0Handshake shake;
		shake.key = "What do you want to do today?";
		p0Handler->enqueue(shake);
	};

	void onTick()
	{
		p0Listener->poll();
		packAll(udp->outgoing);
		unpackAll(udp->incoming);
	};
};