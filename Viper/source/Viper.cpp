#include "Platform.hpp"
#include "log/Logger.hpp"
#include "thread/Threads.hpp"
#include "event/Events.hpp"

std::atomic_bool Viper::running = true;

Viper::Viper()
{
	
};

void Viper::onStart()
{
	viper::platform::setTerminateHandler();

	info("Initializing modules...");

	for (auto&& kv : modules)
	{
		// TODO: dependencies on other modules, insertion order not working?
		if (kv.first != "game")
		{
			kv.second->onStart();
		}
	}
	modules["game"]->onStart();

	set_atom(isInitialized, true, bool);
	info("Initialization complete");
};

void Viper::onShutdown()
{
	info("Shutting down...");

	for (auto&& kv : modules)
	{
		kv.second->onShutdown();
	}
};

void Viper::start()
{
	onStart();

	while(running)
	{
		tickModules();
	}

	onShutdown();
	viper::pollLogger();

	viper::platform::pauseSystem();
};

