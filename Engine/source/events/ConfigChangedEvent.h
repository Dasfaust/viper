#pragma once
#include "../Logger.h"
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

		OnConfigChangedData() { }
		OnConfigChangedData(std::string section, std::string segment, std::vector<boost::variant<int, float, bool, std::string>> values)
			: section(section), segment(segment), values(values)
		{ }
	};

	class OnConfigChanged
	{
	public:
		tbb::concurrent_vector<std::shared_ptr<EventListener<OnConfigChangedData>>> listeners;

		OnConfigChanged() { }

		void addListener(std::shared_ptr<EventListener<OnConfigChangedData>> listener)
		{
			listeners.push_back(listener);
		}

		void triggerEvent(OnConfigChangedData& data)
		{
			for (auto& listener : listeners)
			{
				listener->callback(data);
			}
		}
	};
}