#pragma once
#include "interface/Modular.hpp"
#include "net/NetClient.hpp"

class Client : public Module, public Modular
{
public:
	void onStart() override
	{
		auto nc = initModule<NetClient>("net");

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
