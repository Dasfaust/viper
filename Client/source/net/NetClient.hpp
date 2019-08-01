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
	std::shared_ptr<PacketHandler<P0Telemetry>> p0Handler;
	std::shared_ptr<Listener<P0Telemetry>> p0Listener;

	void onStart() override
	{
		p0Handler = registerPacket<P0Telemetry>(0);
		p0Listener = p0Handler->listen(0, [](P0Telemetry& packet, std::vector<std::shared_ptr<Module>> mods)
		{
			auto nc = std::dynamic_pointer_cast<NetClient>(mods[0]);
			if (packet.serverStatus == 1)
			{
				packet.clientStatus = 1;
				nc->p0Handler->enqueue(packet);
			}
		}, { getParent<Modular>()->getModule("net") });

#ifdef VIPER_WIN64
		udp = std::make_shared<UDPClientWin>();
#endif
		udp->viper = getParent<Module>()->getParent<Viper>();
		udp->address = { "127.0.0.1", 481516 };
		getParent<Module>()->getParent<Modular>()->getModule<Threads>("threads")->watch(udp);
		udp->start();

		P0Telemetry tel;
		tel.clientStatus = 1;
		p0Handler->enqueue(tel);
	};

	void onTick() override
	{
		p0Listener->poll();
		packAll(udp->outgoing);
		unpackAll(udp->incoming);
	};
};