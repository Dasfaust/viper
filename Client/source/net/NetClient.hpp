#pragma once
#include "thread/Threads.hpp"

#ifdef VIPER_WIN64
#include "IPClientWin.hpp"
#endif

class NetClient : public Module, public PacketFactory
{
public:
	std::shared_ptr<IPHandler> ip;

	void onStart() override
	{

#ifdef VIPER_WIN64
		ip = std::make_shared<IPClientWin>();
#endif
		ip->viper = getParent<Module>()->getParent<Viper>();
		ip->factory = shared_from_this();
		ip->address = { "127.0.0.1", 48151, 62342 };
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