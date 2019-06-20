#pragma once
#include "interface/Modular.hpp"
#include "world/World.hpp"
#include "net/Networking.hpp"

class Server : public Module, public Modular
{
public:
	void onStart() override
	{
		initModule<World>("world");
		initModule<Networking>("networking");

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
