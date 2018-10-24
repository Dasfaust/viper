#pragma once
#include "EventListener.h"
#include <string>
#include <memory>
#include <functional>
#include <boost/variant.hpp>
#include <boost/signals2.hpp>
#include "tbb/concurrent_vector.h"

#define OnConfigChanged OnConfigChanged
#define OnConfigChangedData OnConfigChangedData

namespace Event
{
	struct OnConfigChangedData
	{
		std::string section;
		std::string segment;
		std::vector<boost::variant<int, float, bool, std::string>> values;
	};

	class OnConfigChanged
	{
	public:
		std::vector<std::shared_ptr<EventListener<OnConfigChangedData>>> listeners;

		OnConfigChanged() { }

		void addListener(std::shared_ptr<EventListener<OnConfigChangedData>> listener)
		{
			debug("Listener added");
			listeners.emplace_back(listener);
			debug(listeners.size());
		}

		void triggerEvent(std::weak_ptr<OnConfigChangedData> data)
		{
			debug(listeners.size());
			for (auto& listener : listeners)
			{
				debug("Event fired");
				listener->callback(data.lock());
			}
		}
	};
}