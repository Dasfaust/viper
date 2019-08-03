#pragma once
#include "thread/Threads.hpp"

#ifdef VIPER_WIN64
#include "IPServerWin.hpp"
#endif

class NetServer : public Module, public PacketFactory
{
public:
	std::shared_ptr<IPHandler> ip;

	void onStart() override
	{
#ifdef VIPER_WIN64
		ip = std::make_shared<IPServerWin>();
#endif
		ip->viper = getParent<Module>()->getParent<Viper>();
		ip->factory = shared_from_this();
		ip->address = { "", 48151, 62342 };
		getParent<Module>()->getParent<Modular>()->getModule<Threads>("threads")->watch(ip);
		ip->onStart();
		ip->start();
	};

	void onTick() override
	{
		packAll(ip->outgoing);
		unpackAll(ip->incoming);
	};
};