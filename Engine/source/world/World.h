#pragma once
#include "../Module.h"
#include "../Threadable.h"
#include "ECS.h"
#include <unordered_map>
#include <tuple>

class Worker;

class World : public Module, public Threadable
{
public:
	int stepsPerSecond = 0;
	std::vector<std::shared_ptr<Worker>> workers;
	tbb::concurrent_queue<unsigned int> entsToDelete;
	tbb::concurrent_queue<unsigned int> compsToDelete;
	std::shared_ptr<ECS> ecs;

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

	struct Comp : public Component
	{
		unsigned int someInt = 0;
	};

	void onShutdown() override;

	inline void deleteEntity(unsigned int id)
	{
		entsToDelete.emplace(id);
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

class Worker : public Threadable
{
public:
	tbb::concurrent_queue<std::tuple<std::shared_ptr<SystemBase>, unsigned int, unsigned int>> jobs;
	std::atomic<bool> finished = true;
	World* world;

	Worker(World* world)
	{
		this->world = world;
	};

	inline void queueJob(std::tuple<std::shared_ptr<SystemBase>, unsigned int, unsigned int> range)
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

			std::tuple<std::shared_ptr<SystemBase>, unsigned int, unsigned int> job;
			if (jobs.try_pop(job))
			{
				auto system = std::get<0>(job);
				auto comps = world->ecs->getComponents(system->type);

				for (int i = std::get<1>(job); i < std::get<2>(job); i++)
				{
					if (i <= comps.size() - 1)
					{
						system->tickInternal(comps[i]);
					}
					else
					{
						break;
					}
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