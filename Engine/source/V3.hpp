#pragma once
#include "config/ConfigLayer.h"
#include "view/ViewLayer.h"
#include "events/EventLayer.h"

class V3
{
public:
	V3(std::string workingDir);
	~V3();

	EventLayer* getEvents();
	ConfigLayer* getConfig();
	ViewLayer* getView();
private:
	std::shared_ptr<EventLayer> events;
	std::shared_ptr<ConfigLayer> config;
	std::shared_ptr<ViewLayer> view;
};
