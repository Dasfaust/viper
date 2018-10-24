#pragma once
#include "../V3Macros.h"

template<class EventData>
class EventListener
{
public:
	EventListener() { }

	virtual void callback(std::shared_ptr<EventData> data)
	{
		debug("Event fired!");
	}
};