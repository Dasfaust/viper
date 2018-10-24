#pragma once
#include "../V3Macros.h"
#include <memory>
#include <functional>
#include <tbb/concurrent_queue.h>

template<class EventData>
class EventListener
{
public:
	void(*unary)(EventData);
	EventListener(void(*unary)(EventData))
	{
		this->unary = unary;
	}
	virtual ~EventListener() { }

	tbb::concurrent_queue<EventData> queue;

	virtual void callback(std::shared_ptr<EventData> data)
	{
		queue.emplace((*data));
	}

	virtual void poll()
	{
		while (!queue.empty())
		{
			EventData result;
			if (queue.try_pop(result))
			{
				unary(result);
			}
		}
	}
};