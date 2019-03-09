#pragma once
#include "../Macros.h"
#include "Event.h"

class EventLayer
{
public:
	V3API EventLayer();
	V3API ~EventLayer();

	/*tbb::concurrent_unordered_map<unsigned int, std::shared_ptr<EventBase>> events;
	
	template<typename T>
	std::shared_ptr<Event<T>> makeHandler()
	{
		unsigned int id = events.size();
		events[id] = std::make_shared<Event<T>>(id);
		return events[id];
	};

	template<typename T>
	std::shared_ptr<Event<T>> getHandler(unsigned int id)
	{
		return std::static_pointer_cast<Event<T>>(events[id]);
	}*/

	
};