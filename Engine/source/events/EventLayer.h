#pragma once
#include "../Macros.h"
#include "ConfigChangedEvent.h"

class EventLayer
{
public:
	V3API EventLayer();
	V3API ~EventLayer();

	std::shared_ptr<Event::OnConfigChanged> V3API getOnConfigChanged();
private:
	std::shared_ptr<Event::OnConfigChanged> onConfigChanged;
};