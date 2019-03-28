#pragma once
#include <memory>
#include "concurrentqueue.h"

class EventBase
{
public:
};

template<class T>
class EventListener
{
public:
	void(*handle)(T*);
	void(*propegate)(std::shared_ptr<EventBase>, T*);

	EventListener(void(*handle)(T*), void(*propegate)(std::shared_ptr<EventBase>, T*))
	{
		this->handle = handle;
		this->propegate = propegate;
	}
	virtual ~EventListener() { }

	moodycamel::ConcurrentQueue<T*> queue;

	virtual void call(T* data)
	{
		queue.enqueue(data);
	}

	virtual void poll(std::shared_ptr<EventBase> handler)
	{
		T* result;
		while(queue.try_dequeue(result))
		{
			handle(result);
			propegate(handler, result);
		}
	}
};