#pragma once
#include "EventListener.h"
#include "MemoryPool.h"
#include "tbb/concurrent_vector.h"
#include "../Logger.h"
#include <atomic>

class EventData
{
public:
	unsigned int index = 0;
};

template<typename T>
class Event : public EventBase
{
public:
	MemoryPool<T> heap;
	tbb::concurrent_vector<std::shared_ptr<EventListener<T>>> listeners;

	std::shared_ptr<EventListener<T>> listen(void(*func)(T*))
	{
		unsigned int position = listeners.size();
		listeners.resize(position + 1);
		listeners[position] = std::make_shared<EventListener<T>>(func, [](std::shared_ptr<EventBase> event, T* data)
		{
			auto _heap = std::static_pointer_cast<Event<T>>(event)->heap;
			auto _listeners = std::static_pointer_cast<Event<T>>(event)->listeners;
			
			if (data->index >= _listeners.size() - 1)
			{
				_heap.deleteElement(data);
			}

			data->index++;
		});
		return listeners[position];
	};

	T* makeEvent()
	{
		return heap.newElement();
	};

	void fire(T* data)
	{
		if (listeners.size() > 0)
		{
			for (int i = 0; i < listeners.size(); i++)
			{
				listeners[i]->call(data);
			}
		}
	};
};