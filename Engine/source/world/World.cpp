#include "World.h"
#include "../V3.h"
#include "../util/Time.h"
#include "../config/ConfigLayer.h"
#include <future>

World::World()
{

}

World::~World()
{
	delete ecs;
}

void World::onStartup()
{
	ecs = new ECS::Container();

	auto config = v3->getModule<ConfigLayer>();
	stepsPerSecondTarget = config->getInts("engine", "worldStepsPerSecond")[0];
	stepsAsync = config->getBools("engine", "worldStepAsync")[0];
	stepThreadCount = config->getInts("engine", "worldStepThreads")[0];

	debugf("World init: steps: %d, async: %d, threads: %d", stepsPerSecondTarget, stepsAsync, stepThreadCount);

	lastStep = Time::now();
	deltaTime = 1000.0 / stepsPerSecondTarget;

	if (stepsAsync)
	{
		for (unsigned int i = 0; i < stepThreadCount; i++)
		{
			queues.push_back(std::make_shared<moodycamel::ConcurrentQueue<bool>>());
			workers.push_back(std::make_shared<Worker>(this, i));
			workers[i]->start();
			debugf("Worker %d started", i + 1);
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
		debugf("TPS: %d", stepsPerSecond);
    }
    else 
    {
        stepPerformanceAccumulator += stepTime;
    }

    stepAccumulator += stepTime;
    while (stepAccumulator >= deltaTime)
    {
		//debug("Step start");
		if (stepsAsync)
		{
			for (ECS::System* system : ecs->getSystems())
			{
				for (auto kv : system->getTypes())
				{
					unsigned int elements = (ecs->getHeap((&kv)->first).size() / (&kv)->second) - 1;

					debug("### STEP BEGIN");
					std::vector<std::future<bool>> jobs;
					if (elements >= stepThreadCount)
					{
						unsigned int itemsPerWorker = elements / stepThreadCount;
						debugf("Items per worker: %d", itemsPerWorker);
						unsigned int lastIndex = 0;
						for (unsigned int i = 0; i < stepThreadCount; i++)
						{
							int start = lastIndex + (&kv)->second;
							int end = (i + 1 == stepThreadCount ? -1 : (itemsPerWorker == 1 ? start : (lastIndex + (itemsPerWorker * (&kv)->second))));
							lastIndex = end;
							debugf("Worker range: %d - %d", start, end);

							workers[i]->queueJob({ system, (&kv)->first, start, end });
						}
					}
					else
					{
						workers[0]->queueJob({ system, (&kv)->first, -1, -1 });
					}

					for (auto queue : queues)
					{
						queue->enqueue(true);
					}

					for (auto worker : workers)
					{
						bool result;
						while (!worker->finished.try_dequeue(result))
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(1));
						}
					}

					for (unsigned int i = 0; i < ecs->getHeap(1).size(); i += 40)
					{
						auto comp = reinterpret_cast<ECS::TestComponent*>(&ecs->getHeap(1)[i]);
						warnf("Component %d val: %.2f", comp->index, comp->val1);
					}

					while (!entsToDelete.empty())
					{
						unsigned int id;
						if (entsToDelete.try_pop(id))
						{
							ecs->deleteEntity(ecs->getEntity(id));
						}
					}

					debug("### STEP END");
				}
			}
		}
		else
		{
			for (ECS::System* system : ecs->getSystems())
			{
				for (auto kv : system->getTypes())
				{
					tickSystem(system, (&kv)->first);
				}
			}
		}
		//debug("Step end");

        stepAccumulator -= deltaTime;
        stepCount++;
    }

    stepAlpha = stepAccumulator / deltaTime;
}

void World::tickSystem(ECS::System* system, uint8 type, int start, int end)
{
	std::vector<uint32>& heap = ecs->getHeap(type);
	size_t size = system->getTypes()[type];
	for (unsigned int i = start; i < (end > -1 ? end + size : heap.size()); i += size)
	{
		system->tick(deltaTime, reinterpret_cast<ECS::Component*>(&heap[i]), ecs);
	}
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
}