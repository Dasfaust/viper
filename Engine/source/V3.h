#pragma once
#include "config/ConfigLayer.h"
#include "view/ViewLayer.h"
#include "events/EventLayer.h"
#include "Tickable.h"

class V3
{
public:
	V3(std::string workingDir);
	~V3();

	std::shared_ptr<EventLayer> getEvents();
	std::shared_ptr<ConfigLayer> getConfig();
	std::shared_ptr<ViewLayer> getView();

	void start();
	void addToLogicTicks(std::shared_ptr<Tickable> object);
	void addToRenderTicks(std::shared_ptr<Tickable> object);
private:
	std::shared_ptr<EventLayer> events;
	std::shared_ptr<ConfigLayer> config;
	std::shared_ptr<ViewLayer> view;
	Tickable::TicArray renderTicks;
	Tickable::TicArray logicTicks;
};
