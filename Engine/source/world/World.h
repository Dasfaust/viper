#pragma once
#include "../Module.h"
#include "../Threadable.h"
#include "ECS.h"
#include <unordered_map>
#include <tuple>

class Worker : public Threadable
{
public:
	tbb::concurrent_queue<std::tuple<unsigned int, unsigned int>> jobs;
	std::atomic<bool> finished = true;

	inline void queueJob(std::tuple<unsigned int, unsigned int> range)
	{
		jobs.emplace(range);
	};

	void tick() override
	{
		if (!jobs.empty())
		{
			if (finished)
			{
				bool expected = false;
				while (!finished.compare_exchange_weak(expected, false));
			}

			std::tuple<unsigned int, unsigned int> job;
			if (jobs.try_pop(job))
			{
				for (int i = std::get<0>(job); i < std::get<1>(job); i++)
				{
					
					/*if (i <= ECS::componentIds.size() - 1)
					{
						unsigned int id = ECS::componentIds[i];
						ECS::components[id]->tick_func(id);
					}
					else
					{
						break;
					}*/
				}
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 30));
		}

		if (!finished)
		{
			bool expected = true;
			while (!finished.compare_exchange_weak(expected, true));
		}
	};
};

class World : public Module, public Threadable
{
public:
	int stepsPerSecond = 0;
	std::vector<std::shared_ptr<Worker>> workers;
	tbb::concurrent_queue<unsigned int> entsToDelete;
	tbb::concurrent_queue<unsigned int> compsToDelete;

	V3API World();
	V3API ~World();

	void onStartup() override;
	void tick() override;

	inline void onTick() override
	{
		if (!stepsAsync)
		{
			this->tick();
		}
	};

	void onShutdown() override;

	inline void deleteEntity(unsigned int id)
	{
		entsToDelete.emplace(id);
	};

	inline void deleteComponent(unsigned int id)
	{
		compsToDelete.emplace(id);
	};
private:
    int stepsPerSecondTarget;
    bool stepsAsync = false;
    int stepThreadCount = 1;

    double stepAlpha = 0.0;
    int stepCount = 0;
    double lastStep = 0.0;
    double stepTime = 0.0;
    double deltaTime = 0.0;
    double stepAccumulator = 0.0;
    double stepPerformanceAccumulator = 0.0;
};