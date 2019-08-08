#pragma once
#include "../util/Memory.hpp"
#include "../interface/Modular.hpp"
#include "../util/Future.hpp"
#include <map>
#include <concurrentqueue.h>

struct Event
{
	bool cancelled = false;
};

template<typename T>
struct EventWrapper
{
	T event;
	Future<T>* future;
};

template<typename T>
class Listener
{
public:
	uint8 position;
	moodycamel::ConcurrentQueue<T> events;
	moodycamel::ConcurrentQueue<EventWrapper<T>> eventFutures;
	std::vector<std::shared_ptr<Module>> mods;
	void(*onEvent)(T&, std::vector<std::shared_ptr<Module>>);
	std::shared_ptr<Listener<T>> next;

	Listener(uint8 position, std::vector<std::shared_ptr<Module>> mods, void(*onEvent)(T&, std::vector<std::shared_ptr<Module>>))
	{
		this->position = position;
		this->mods = mods;
		this->onEvent = onEvent;
	}

	void poll()
	{
		T ev;
		while(events.try_dequeue(ev))
		{
			onEvent(ev, mods);

			if (!ev.cancelled && next != 0)
			{
				next->events.enqueue(ev);
			}
		}

		EventWrapper<T> ew;
		while (eventFutures.try_dequeue(ew))
		{
			onEvent(ew.event, mods);

			if (!ew.event.cancelled && next != 0)
			{
				next->eventFutures.enqueue(ew);
			}
			else
			{
				ew.future->post(ew.event);
			}
		}
	};
};

class Events : public Module, public Modular
{
public:
};

template<typename T>
class EventHandler : public Module
{
public:
	std::map<uint8, std::shared_ptr<Listener<T>>> listeners;

	std::shared_ptr<Listener<T>> listen(uint8 position, void(*onEvent)(T&, std::vector<std::shared_ptr<Module>>), std::vector<std::shared_ptr<Module>> mods = { })
	{
		listeners[position] = std::make_shared<Listener<T>>(position, mods, onEvent);

		std::vector<std::shared_ptr<Listener<T>>> lis;
		for (auto&& kv : listeners)
		{
			lis.push_back(kv.second);
		}

		for (uint32 i = 0; i < lis.size(); i++)
		{
			if (i != 0 && lis[i]->position == position)
			{
				lis[i - 1]->next = lis[i];
				break;
			}
		}

		return listeners[position];
	};

	void fire(T ev)
	{
		if (!listeners.empty())
		{
			listeners.begin()->second->events.enqueue(ev);
		}
	};

	void fire(T ev, Future<T>* future)
	{
		if (!listeners.empty())
		{
			listeners.begin()->second->eventFutures.enqueue({ ev, future });
		}
		else
		{
			future->post(ev);
		}
	};
};