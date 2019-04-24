#pragma once
#include "Time.h"
#include "../Module.h"
#include <unordered_map>
#include "concurrentqueue.h"
#include "../Logger.h"

#define profiler_begin(x) if (v3->isModuleLoaded<Profiler>()) v3->getModule<Profiler>()->begin(x);
#define profiler_end(x) if (v3->isModuleLoaded<Profiler>()) v3->getModule<Profiler>()->end(x);

struct Change
{
	std::string key;
	double val;
};

class Profiler : public Module
{
public:
	std::unordered_map<std::string, double> startpoints;
	std::unordered_map<std::string, double> endpoints;
	std::unordered_map<std::string, double> durations;
	moodycamel::ConcurrentQueue<Change> starting;
	moodycamel::ConcurrentQueue<Change> ending;

	inline void begin(std::string key)
	{
		starting.enqueue({ key, tnow() });
	}

	inline void end(std::string key)
	{
		ending.enqueue({ key, tnow() });
	}

	inline void tickWait() override
	{
		Change change;
		while(starting.try_dequeue(change))
		{
			startpoints[change.key] = change.val;
		}

		while (ending.try_dequeue(change))
		{
			endpoints[change.key] = change.val;
		}

		for (auto it = startpoints.begin(); it != startpoints.end();)
		{
			if (endpoints.count(it->first))
			{
				durations[it->first] = endpoints[it->first] - it->second;
				endpoints.erase(it->first);
				startpoints.erase(it->first);
			}
			it++;
		}
	};

	inline void onTick() override
	{
		for (auto&& kv : durations)
		{
			debugf("#?# [%s] took %.2fms", kv.first.c_str(), kv.second);
		}
	};
};