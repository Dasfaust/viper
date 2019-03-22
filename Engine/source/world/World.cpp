#include "World.h"
#include "../V3.h"
#include "../util/Time.h"
#include "../config/ConfigLayer.h"

World::World()
{

}

World::~World()
{

}

void World::onStartup()
{
	ecs = std::make_shared<ECS>();

	auto config = v3->getModule<ConfigLayer>();
	stepsPerSecondTarget = config->getInts("engine", "worldStepsPerSecond")[0];
	stepsAsync = config->getBools("engine", "worldStepAsync")[0];
	stepThreadCount = config->getInts("engine", "worldStepThreads")[0];

	debugf("World init: steps: %d, async: %d, threads: %d", stepsPerSecondTarget, stepsAsync, stepThreadCount);

	lastStep = Time::now();
	deltaTime = 1000.0 / stepsPerSecondTarget;

	if (stepsAsync)
	{
		for (int i = 0; i < stepThreadCount; i++)
		{
			workers.push_back(std::make_shared<Worker>(this));
			workers[i]->start();
			debugf("Worker %d started", i);
		}

		debugf("Starting world (async)");
		this->start();
	}
	else
	{
		debugf("Starting world");
	}
}

void World::tick()
{
    double newStep = Time::now();
    stepTime = newStep - lastStep;
    lastStep = newStep;

    if (stepPerformanceAccumulator >= 1000.0)
    {
        stepsPerSecond = stepCount;
        stepPerformanceAccumulator = 0;
        stepCount = 0;
    }
    else 
    {
        stepPerformanceAccumulator += stepTime;
    }

    stepAccumulator += stepTime;
    while (stepAccumulator >= deltaTime)
    {
		if (stepsAsync)
		{
			auto systems = ecs->getSystems();

			for (auto kv : systems)
			{
				for (auto system : (&kv)->second)
				{
					auto comps = ecs->getComponents(system->type);

					if (comps.size() >= stepThreadCount)
					{
						unsigned int itemsPerWorker = comps.size() / stepThreadCount;
						for (unsigned int i = 0; i < stepThreadCount; i++)
						{
							unsigned int start = i * itemsPerWorker + 1;
							workers[i]->queueJob(std::make_tuple(system, start, start + itemsPerWorker));
						}

						for (auto worker : workers)
						{
							while (!worker->finished);
						}

						if (!entsToDelete.empty())
						{
							unsigned int id;
							if (entsToDelete.try_pop(id))
							{
								ecs->deleteEntity(id);
							}
						}
					}
					else
					{
						for (auto comp : comps)
						{
							system->tickInternal(comp);
						}
					}
				}

				/*auto comps = ecs->getComponentIds(sys->type);

				if (comps.size() >= stepThreadCount)
				{
					unsigned int itemsPerWorker = comps.size() / stepThreadCount;
					for (unsigned int i = 0; i < stepThreadCount; i++)
					{
						unsigned int start = i * itemsPerWorker + 1;
						workers[i]->queueJob(std::make_tuple(sys->id, start, start + itemsPerWorker));
					}

					for (auto worker : workers)
					{
						while (!worker->finished);
					}

					if (!entsToDelete.empty())
					{
						unsigned int id;
						if (entsToDelete.try_pop(id))
						{
							ecs->deleteEntity(id);
						}
					}
				}
				else
				{
					for (auto comp : comps)
					{
						sys->tickInternal(ecs->getComponent(sys->type, comp));
					}
				}*/
			}
		}
		else
		{
			auto systems = ecs->getSystems();

			for (auto kv : systems)
			{
				for (auto system : (&kv)->second)
				{
					auto comps = ecs->getComponents(system->type);

					for (auto comp : comps)
					{
						system->tickInternal(comp);
					}
				}
			}

			/*for (auto sys : systems)
			{
				auto comps = ecs->getComponents(sys->type);

				for (auto comp : comps)
				{
					sys->tickInternal(comp);
				}
			}*/
		}

        stepAccumulator -= deltaTime;
        stepCount++;
    }

    stepAlpha = stepAccumulator / deltaTime;
}

void World::onShutdown()
{
	if (stepsAsync)
	{
		this->stop();
		for (auto worker : workers)
		{
			worker->stop();
		}
	}

	static_cast<void>(ecs->purge());
}