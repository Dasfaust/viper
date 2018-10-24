#pragma once
#include "ConfigChangedEvent.h"

class EventLayer
{
public:
	EventLayer();
	~EventLayer();

	std::shared_ptr<Event::OnConfigChanged> getOnConfigChanged();
private:
	std::shared_ptr<Event::OnConfigChanged> onConfigChanged;
};