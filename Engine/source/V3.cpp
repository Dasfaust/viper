#include "V3.h"
#include <memory>
#include "world/ECS.h"-

V3::V3()
{
	std::cout << "V3**[init] Starting logger..." << std::endl;
	Log::start();

	dt = initModule<DeltaTime>(1000.0);
}

V3::~V3()
{
	Log::stop();
}

void V3::start()
{
	debugf("Num modules: %d", modules.size());

	debug("Starting up...");
	for (auto element : modules)
	{
		element.second->onStartup();
	}

	debug("Starting main loop");
	while (running)
	{
		for (std::pair<unsigned int, Module*> element : modules)
		{
			element.second->onTickBegin();
		}

		for (std::pair<unsigned int, Module*> element : modules)
		{
			element.second->onPreRender();
		}

		for (std::pair<unsigned int, Module*> element : modules)
		{
			if (element.second->intervalAccumulator >= element.second->interval)
			{
				element.second->intervalAccumulator = 0.0;
				element.second->onTick();
			}
			else
			{
				element.second->intervalAccumulator += dt->deltaTime;
				element.second->tickWait();
			}
		}

		for (std::pair<unsigned int, Module*> element : modules)
		{
			element.second->onPostRender();
		}

		for (std::pair<unsigned int, Module*> element : modules)
		{
			element.second->onTickEnd();
		}
	}

	info("Shutting down...");

	for (std::pair<unsigned int, Module*> element : modules)
	{
		element.second->onShutdown();
	}

	static_cast<void>(modules.empty());

	info("Complete. Goodbye!");
}