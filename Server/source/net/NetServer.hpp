#pragma once
#include "Packets.hpp"
#include "thread/Threads.hpp"

#ifdef VIPER_WIN64
#include "UDPServerWin.hpp"
#endif

class NetServer : public Module, public PacketFactory
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
			debug("Client id is %s", boost::lexical_cast<std::string>(packet.client).c_str());
			if (packet.key == "What do you want to do today?")
			{
				P0Handshake shake;
				shake.key = "The same thing we do every night, Pinky. Try to take over the world!";
				std::dynamic_pointer_cast<NetServer>(mods[0])->p0Handler->enqueue(shake, { packet.client });
			}
		}, { getParent<Modular>()->getModule("net") });

#ifdef VIPER_WIN64
		udp = std::make_shared<UDPServerWin>();
#endif
		udp->address = { "", 481516 };
		getParent<Module>()->getParent<Modular>()->getModule<Threads>("threads")->watch(udp);
		udp->start();
	};

	void onTick() override
	{
		p0Listener->poll();
		packAll(udp->outgoing);
		unpackAll(udp->incoming);
	};
};