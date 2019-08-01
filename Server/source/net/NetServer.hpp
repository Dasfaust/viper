#pragma once
#include "thread/Threads.hpp"

#ifdef VIPER_WIN64
#include "UDPServerWin.hpp"
#endif

class NetServer : public Module, public PacketFactory
{
public:
	std::shared_ptr<UDP> udp;

	void onStart() override
	{
#ifdef VIPER_WIN64
		udp = std::make_shared<UDPServerWin>();
#endif
		udp->viper = getParent<Module>()->getParent<Viper>();
		udp->address = { "", 481516 };
		getParent<Module>()->getParent<Modular>()->getModule<Threads>("threads")->watch(udp);
		udp->start();
	};

	void onTick() override
	{
		packAll(udp->outgoing);
		unpackAll(udp->incoming);
	};
};