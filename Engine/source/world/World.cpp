#include "World.h"
#include "../V3.h"
#include "../util/Time.h"
#include "../config/ConfigLayer.h"
#include "systems/MovementInputSystem.h"

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
	Components::registerTypes(ecs);

	auto config = v3->getModule<ConfigLayer>();
	stepsPerSecondTarget = config->getInts("engine", "worldStepsPerSecond")[0];
	stepsAsync = config->getBools("engine", "worldStepAsync")[0];
	stepThreadCount = config->getInts("engine", "worldStepThreads")[0];

	debugf("World init: steps: %d, async: %d, threads: %d", stepsPerSecondTarget, stepsAsync, stepThreadCount);

	lastStep = Time::now();
	deltaTime = 1000.0 / stepsPerSecondTarget;

	if (stepsAsync)
	{
		for (unsigned int i = 0; i < (unsigned int)stepThreadCount; i++)
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
        stepsPerSecond = (stepCount > stepsPerSecondTarget ? (stepCount - (stepCount - stepsPerSecondTarget)) : stepCount);
        stepPerformanceAccumulator = 0;
        stepCount = 0;
		stepTarget = ((float)stepsPerSecondTarget / (float)stepsPerSecond) * 100.0f;
		/*debugf("Systems: %d", ecs->getSystems().size());
		auto type = ecs->resolveType<ECS::Entity>();
		auto& entHeap = ecs->getHeap(0);
		unsigned int numEnts = entHeap.size() / type.size;
		debugf("Entities: %d", numEnts);
		if (numEnts > 1)
		{
			for (uint32 i = type.size; i < entHeap.size(); i += type.size)
			{
				ECS::Entity* ent = reinterpret_cast<ECS::Entity*>(&entHeap[i]);
				debugf(" -> Entity %d components: %d", i, ent->components.size());
				for (auto kv : ent->components)
				{
					auto comp = ecs->getComponent(ent, (&kv)->first);
					debugf("  -> Component %d type %d", comp->index, comp->type_id);
				}
			}
		}*/
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
					unsigned int elements = (unsigned int)((ecs->getHeap((&kv)->first).size() / (&kv)->second) - 1);

					//debug("### STEP BEGIN");
					if (elements >= (unsigned int)stepThreadCount)
					{
						unsigned int itemsPerWorker = elements / stepThreadCount;
						//debugf("Items per worker: %d", itemsPerWorker);
						unsigned int lastIndex = 0;
						for (unsigned int i = 0; i < (unsigned int)stepThreadCount; i++)
						{
							int start = (int)(lastIndex + (&kv)->second);
							int end = (i + 1 == stepThreadCount ? -1 : (itemsPerWorker == 1 ? start : (lastIndex + (itemsPerWorker * (unsigned int)(&kv)->second))));
							lastIndex = end;
							//debugf("Worker range: %d - %d", start, end);

							workers[i]->queueJob({ system, (&kv)->first, start, end });
						}
					}
					else
					{
						workers[0]->queueJob({ system, (&kv)->first, 0, -1 });
						//debugf("Ticking all");
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

					/*numTicks++;
					for (unsigned int i = 0; i < ecs->getHeap(1).size(); i += 40)
					{
						auto comp = reinterpret_cast<ECS::TestComponent*>(&ecs->getHeap(1)[i]);
						if (comp->val1 != numTicks)
						{
							critf("Component %d has unexpected value: %.2f", comp->index, comp->val1);
						}
					}*/

					//debug("### STEP END");
				}
			}
		}
		else
		{
			debug("Ticking systems (sync)");
			for (ECS::System* system : ecs->getSystems())
			{
				for (auto kv : system->getTypes())
				{
					tickSystem(system, (&kv)->first, 0, -1);
				}
			}
		}
		//debug("Step end");

		std::pair<uint32, std::vector<ECS::TypeInfo>> sys;
		while (systemsToCreate.try_dequeue(sys))
		{
			auto s = ecs->createSystem(sys.second[0], sys.first, v3, this);
			for (ECS::TypeInfo type : sys.second)
			{
				if (type.id != sys.second[0].id)
				{
					s->addType(type.id, type.size);
				}
			}
		}

		std::shared_ptr<Future<ECS::Entity*>> entFuture;
		while (entsToCreate.try_dequeue(entFuture))
		{
			auto entity = ecs->getEntity(ecs->createEntity());
			for (uint32 i = 0; i < entFuture->components.size(); i++)
			{
				ECS::TypeInfo info = entFuture->components[i];
				debugf("Created component: type %d, size %d", info.id, info.size);
				uint32 id = ecs->createComponent(entity, info.id, info.size);
				if (entFuture->callback != nullptr)
				{
					entFuture->callback(i, ecs->getComponent(entity, info.id));
				}
			}
			entFuture->fulfill(entity);
		}

		ECS::Entity* entity;
		while (entsToDelete.try_dequeue(entity))
		{
			ecs->deleteEntity(entity);
		}

		ECS::Component* component;
		while (compsToDelete.try_dequeue(component))
		{
			ecs->deleteComponent(component);
		}

        stepAccumulator -= deltaTime;
        stepCount++;
    }

	for (ECS::System* system : ecs->getSystems())
	{
		system->tickWait(system, this);
	}

    stepAlpha = stepAccumulator / deltaTime;
}

void World::tickSystem(ECS::System* system, uint8 type, int start, int end)
{
	std::vector<uint32>& heap = ecs->getHeap(type);
	size_t size = system->getTypes()[type];
	for (unsigned int i = start; i < (end > -1 ? end + size : (uint32)heap.size()); i += (unsigned int)size)
	{
		if (i == 0) continue;
		system->tickFunc(deltaTime, reinterpret_cast<ECS::Component*>(&heap[i]), system, ecs, this);
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