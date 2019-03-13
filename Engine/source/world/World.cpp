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
	ECS::init();

	auto config = v3->getModule<ConfigLayer>();
	stepsPerSecondTarget = config->getInts("engine", "worldStepsPerSecond")[0];
	stepsAsync = config->getBools("engine", "worldStepAsync")[0];
	stepThreadCount = config->getInts("engine", "worldStepThreads")[0];

	debugf("World init: steps: %d, async: %d, threads: %d", stepsPerSecondTarget, stepsAsync, stepThreadCount);

	lastStep = Time::now();
	deltaTime = 1000.0 / stepsPerSecondTarget;

	/*class SomeComp : public Component<SomeComp>
	{
	public:
		int someInt = 10;
	};

	class Ticker : public ComponentTicker<SomeComp>
	{
	public:
		void tick(SomeComp* comp) override
		{
			comp->someInt = rand();
		};
	};
	auto ticker = std::make_shared<Ticker>();
	auto system = ECS::makeSystem<SomeComp>(ticker);
	auto comp = system->makeComponent();
	unsigned int id = comp->id;
	SomeComp* get = system->getComponent(id);
	debugf("SomeComp: %d, type: %d", get->someInt, get->type_id);

	auto ent = ECS::makeEntity();
	debugf("Entity ID: %d", ent->id);
	ECS::deleteEntity(ent);
	ent = ECS::makeEntity();
	debugf("Entity ID: %d", ent->id);
	auto ent2 = ECS::makeEntity();
	debugf("Entity ID: %d", ent2->id);

	ECS::addComponentToEntity<SomeComp>(ent, get);
	for (auto c : ECS::getEntityComponents(ent))
	{
		debugf("Entity (%d) component type: %d", ent->id, c->type_id);
	}

	system->deleteComponent(comp);
	for (auto c : ECS::getEntityComponents(ent))
	{
		debugf("Entity (%d) component type: %d", ent->id, c->type_id);
	}

	comp = system->makeComponent();
	ECS::addComponentToEntity<SomeComp>(ent, comp);
	ECS::deleteEntity(ent);
	debugf("Num components: %d", ECS::components.size());
	debugf("Num comp->ent: %d", ECS::compToEnt.size());
	debugf("Num ent->comp: %d", ECS::entToComp.size());

	for (int i = 0; i < 800000; i++)
	{
		auto e = ECS::makeEntity();
		auto c = system->makeComponent();
		ECS::addComponentToEntity<SomeComp>(e, c);
	}

	debugf("Num entities: %d", ECS::entities.size());
	debugf("Num components: %d", ECS::components.size());
	debugf("Num comp->ent: %d", ECS::compToEnt.size());
	debugf("Num ent->comp: %d", ECS::entToComp.size());*/

	if (stepsAsync)
	{
		for (int i = 0; i < stepThreadCount; i++)
		{
			workers.push_back(std::make_shared<Worker>());
			workers[i]->start();
			debugf("Worker %d started", i);
		}

		debugf("Starting world async");
		this->start();
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
		debugf("World TPS: %d", stepsPerSecond);
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
			if (ECS::componentIds.size() > 1000)
			{
				unsigned int itemsPerWorker = ECS::components.size() / stepThreadCount;
				for (unsigned int i = 0; i < stepThreadCount; i++)
				{
					unsigned int start = i * itemsPerWorker + 1;
					workers[i]->queueJob(std::make_tuple(start, start + itemsPerWorker));
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
						ECS::deleteEntity(ECS::getEntity(id));
					}
				}

				if (!compsToDelete.empty())
				{
					unsigned int id;
					if (compsToDelete.try_pop(id))
					{
						auto comp = ECS::components[id];
						if (ECS::compToEnt.count(id))
						{
							for (auto ent : ECS::compToEnt[id])
							{
								if (ECS::entToComp.count(ent->id))
								{
									auto list = ECS::entToComp[ent->id];
									std::iter_swap(list.begin(), list.end() - 1);
									list.pop_back();
									ECS::entToComp[ent->id] = list;
								}
							}
						}
						comp->del_func(comp);
					}
				}
			}
			else
			{
				for (auto id : ECS::componentIds)
				{
					auto comp = ECS::components[id];
					if (!comp->is_client)
					{
						comp->tick_func(id);
					}
				}
			}
		}
		else
		{
			for (auto id : ECS::componentIds)
			{
				auto comp = ECS::components[id];
				if (!comp->is_client)
				{
					comp->tick_func(id);
				}
			}
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
}