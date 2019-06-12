#pragma once
#include "../util/Memory.hpp"
#include "../interface/Modular.hpp"
#include <map>
#include <concurrentqueue.h>

class Viper;

struct Event
{
	bool cancelled = false;
};

template<typename T>
class Listener
{
public:
	uint8 position;
	moodycamel::ConcurrentQueue<T> events;
	Viper* viper;
	void(*onEvent)(T&, Viper*);
	std::shared_ptr<Listener<T>> next;

	Listener(uint8 position, Viper* viper, void(*onEvent)(T&, Viper*))
	{
		this->position = position;
		this->viper = viper;
		this->onEvent = onEvent;
	}

	void poll()
	{
		T ev;
		while(events.try_dequeue(ev))
		{
			onEvent(ev, viper);

			if (!ev.cancelled && next != 0)
			{
				next->events.enqueue(ev);
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

	std::shared_ptr<Listener<T>> listen(uint8 position, void(*onEvent)(T&, Viper*))
	{
		listeners[position] = std::make_shared<Listener<T>>(position, getParent<Events>()->getParent<Viper>(), onEvent);

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
		listeners.begin()->second->events.enqueue(ev);
	};
};