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
		kv.second->onStart();
	}

	info("Initialization complete");
};

void Viper::onShutdown()
{
	info("Shutting down...");

	for (auto&& kv : modules)
	{
		kv.second->onShutdown();
	}

	viper::pollLogger();

	viper::platform::pauseSystem();
};

void Viper::start()
{
	onStart();

	while(running)
	{
		tickModules();
	}

	onShutdown();
};

