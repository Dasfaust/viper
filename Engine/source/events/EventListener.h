#pragma once
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

	virtual void callback(EventData& data)
	{
		queue.emplace(data);
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