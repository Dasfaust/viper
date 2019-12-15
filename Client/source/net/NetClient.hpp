#pragma once
#include "thread/Threads.hpp"
#include "Viper.hpp"

#ifdef VIPER_WIN64
#include "IPClientWin.hpp"
#endif

class NetClient : public Module, public PacketFactory
{
public:
	std::shared_ptr<IPHandler> ip;
	float tickTimeMs;
	
	void onStart() override
	{

#ifdef VIPER_WIN64
		ip = std::make_shared<IPClientWin>();
		ip->sleep = false;
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
		auto start = Time::now();
		packAll(ip->outgoing);
		unpackAll(ip->incoming);
		tickTimeMs = Time::toMilliseconds(Time::since(start));
	};
};