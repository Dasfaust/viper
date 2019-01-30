#include "V3.h"
#include <memory>
#include "Logger.h"
#include "pipeline/PipelineOpenGL.h"

V3::V3(std::string workingDir)
{
	Log::start();
	events = std::make_shared<EventLayer>();
	config = std::make_shared<ConfigLayer>(workingDir, events);
	view = std::make_shared<ViewLayer>(events, config);

	if (config->getStrings("engine", "renderer")[0] == "opengl")
	{
		pipeline = std::make_shared<PipelineOpenGL>(events, config, view);
	}
	else
	{
		std::string message = "No valid rendering API selected.";
		crit(message);
		throw std::exception();
	}

	addToRenderTicks(view);
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
	while (!view->closeRequested())
	{
		view->tick();

		for (auto tickable : renderTicks)
		{
			tickable->tick();
		}

		pipeline->tick();
	}

	info("Shutting down, goodbye.");
	renderTicks.empty();
	logicTicks.empty();
}

void V3::addToLogicTicks(std::shared_ptr<Tickable> object)
{
	logicTicks.push_back(object);
}

void V3::addToRenderTicks(std::shared_ptr<Tickable> object)
{
	renderTicks.push_back(object);
}
