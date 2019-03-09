#pragma once
#include <memory>
#include <tbb/concurrent_queue.h>

class EventBase
{
public:
};

template<class T>
class EventListener : public EngineExtension
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

	tbb::concurrent_queue<T*> queue;

	virtual void call(T* data)
	{
		queue.emplace(data);
	}

	virtual void poll(std::shared_ptr<EventBase> handler)
	{
		while (!queue.empty())
		{
			T* result;
			if (queue.try_pop(result))
			{
				handle(result);
				propegate(handler, result);
			}
		}
	}
};