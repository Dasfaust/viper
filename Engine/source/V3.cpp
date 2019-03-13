#include "V3.h"
#include <memory>
#include "world/ECS.h"-

/*std::shared_ptr<V3> V3::instance = 0;

std::shared_ptr<V3> V3::getInstance()
{
	if (instance == 0)
	{
		instance = std::make_shared<V3Container>();
	}

	return instance;
}*/

V3::V3()
{
	std::cout << "V3** Starting logger..." << std::endl;
	Log::start();

	dt = initModule<DeltaTime>(1000.0);

	tickables = std::make_shared<TickMap>();
	//config = std::make_shared<ConfigLayer>(FileUtils::getWorkingDirectory() + FileUtils::getPathSeperator() + "resources");

	/*if (config->getStrings("engine", "renderer")[0] == "opengl")
	{
		view = std::make_shared<ViewLayer>();
		pipeline = std::make_shared<PipelineOpenGL>(config, view);
		addTickable(view);
		debugWindowTitle = "V3: OpenGL";
		view->setTitle(debugWindowTitle);
	}
	else if (config->getStrings("engine", "renderer")[0] == "software")
	{
		// init olc
	}
	else
	{
		std::string message = "No valid rendering API selected.";
		crit(message);
		throw std::exception();
	}*/
}

V3::~V3()
{
	Log::stop();
}

/*std::shared_ptr<ConfigLayer> V3::getConfig()
{
	return config;
}

std::shared_ptr<ViewLayer> V3::getView()
{
	return view;
}

std::shared_ptr<Pipeline> V3::getPipeline()
{
	return pipeline;
}*/

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

	static_cast<void>(tickables->empty());
	static_cast<void>(modules.empty());
	static_cast<void>(ECS::purge());

	info("Complete. Goodbye!");
}

int V3::addTickable(std::shared_ptr<Tickable> object)
{
	int id = (int)tickables->size();
	(*tickables)[id] = object;
	return id;
}

/*int V3::addExtension(std::shared_ptr<EngineExtension> object, double interval)
{
	object->interval = interval;
	int id = (int)extensions->size();
	(*extensions)[id] = object;
	return id;
}*/