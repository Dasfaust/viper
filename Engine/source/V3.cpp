#include "V3.h"
#include <memory>
#include "Logger.h"

V3::V3(std::string workingDir)
{
	Log::start();
	events = std::make_shared<EventLayer>();
	config = std::make_shared<ConfigLayer>(workingDir, events);
	view = std::make_shared<ViewLayer>(events, config);

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

void V3::start()
{
	while (!view->closeRequested())
	{
		for (auto tickable : renderTicks)
		{
			tickable->tick();
		}
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
