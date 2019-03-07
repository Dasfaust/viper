#include "V3.h"
#include <memory>
#include "Logger.h"
#include "pipeline/PipelineOpenGL.h"
#include "util/Time.h"
#include "util/FileUtils.h"
#include "Memory.h"

std::shared_ptr<V3> V3::instance = 0;

std::shared_ptr<V3> V3::getInstance()
{
	if (instance == 0)
	{
		instance = std::make_shared<V3Container>();
	}

	return instance;
}

V3::V3()
{
	std::cout << "V3 init" << std::endl;
	Log::start();
	events = std::make_shared<EventLayer>();
	config = std::make_shared<ConfigLayer>(FileUtils::getWorkingDirectory() + FileUtils::getPathSeperator() + "resources", events);
	tickables = std::make_shared<TickMap>();
	extensions = std::make_shared<ExtensionMap>();

	if (config->getStrings("engine", "renderer")[0] == "opengl")
	{
		view = std::make_shared<ViewLayer>(events, config);
		pipeline = std::make_shared<PipelineOpenGL>(config, view, events);
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
	}
}

V3::~V3()
{
	Log::stop();
}

std::shared_ptr<EventLayer> V3::getEvents()
{
	return events;
}

std::shared_ptr<ConfigLayer> V3::getConfig()
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
}

void V3::start()
{
	for (std::pair<unsigned int, std::shared_ptr<EngineExtension>> element : (*extensions))
	{
		element.second->onStartup();
	}

	while (!view->closeRequested())
	{
		for (std::pair<unsigned int, std::shared_ptr<EngineExtension>> element : (*extensions))
		{
			element.second->onTickBegin();
		}

		for (std::pair<unsigned int, std::shared_ptr<Tickable>> element : (*tickables))
		{
			element.second->tick();
		}

		for (std::pair<unsigned int, std::shared_ptr<EngineExtension>> element : (*extensions))
		{
			element.second->onPreRender();
		}

		view->tick();

		pipeline->deltaTime = deltaTime;
		pipeline->tick();

		for (std::pair<unsigned int, std::shared_ptr<EngineExtension>> element : (*extensions))
		{
			element.second->onPostRender();
			
			if (element.second->intervalAccumulator >= element.second->interval)
			{
				element.second->intervalAccumulator = 0.0;
				element.second->onTick();
			}
			else
			{
				element.second->intervalAccumulator += deltaTime;
				element.second->tickWait();
			}
		}

		for (std::pair<unsigned int, std::shared_ptr<EngineExtension>> element : (*extensions))
		{
			element.second->onTickEnd();
		}

	}

	info("Shutting down, goodbye.");
	static_cast<void>(tickables->empty());
	static_cast<void>(extensions->empty());

	Memory::purge();
}

int V3::addTickable(std::shared_ptr<Tickable> object)
{
	int id = (int)tickables->size();
	(*tickables)[id] = object;
	return id;
}

int V3::addExtension(std::shared_ptr<EngineExtension> object, double interval)
{
	object->interval = interval;
	int id = (int)extensions->size();
	(*extensions)[id] = object;
	return id;
}