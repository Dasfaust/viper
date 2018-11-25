#pragma once
#include "Macros.h"
#include "config/ConfigLayer.h"
#include "view/ViewLayer.h"
#include "events/EventLayer.h"
#include "Tickable.h"

class V3
{
public:
	V3API V3(std::string workingDir);
	V3API ~V3();

	static std::string V3API getWorkingDirectory();

	std::shared_ptr<EventLayer> V3API getEvents();
	std::shared_ptr<ConfigLayer> V3API getConfig();
	std::shared_ptr<ViewLayer> V3API getView();

	void V3API start();
	void V3API addToLogicTicks(std::shared_ptr<Tickable> object);
	void V3API addToRenderTicks(std::shared_ptr<Tickable> object);
private:
	std::shared_ptr<EventLayer> events;
	std::shared_ptr<ConfigLayer> config;
	std::shared_ptr<ViewLayer> view;
	Tickable::TicArray renderTicks;
	Tickable::TicArray logicTicks;
};
