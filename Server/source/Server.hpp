#pragma once
#include "interface/Modular.hpp"
#include "world/World.hpp"
#include "net/NetServer.hpp"

class Server : public Module, public Modular
{
public:
	void onStart() override
	{
		auto wo = initModule<World>("world");
		auto ns = initModule<NetServer>("net");

		for (auto&& kv : modules)
		{
			kv.second->onStart();
		}
	};

	void onTick() override
	{
		tickModules();
	};

	void onShutdown() override
	{
		for (auto&& kv : modules)
		{
			kv.second->onShutdown();
		}
	};
};
