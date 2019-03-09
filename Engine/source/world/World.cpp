#include "World.h"
#include "../V3.h"
#include "../util/Time.h"

std::shared_ptr<World> World::instance = 0;

std::shared_ptr<World> World::getInstance()
{
	if (instance == 0)
	{
		instance = std::make_shared<WorldContainer>();
	}

	return instance;
}

World::World()
{
	ECS::init();
}

World::~World()
{

}

void World::onStartup()
{
	auto config = V3::getInstance()->getConfig();
	stepsPerSecondTarget = config->getInts("engine", "worldStepsPerSecond")[0];
	stepsAsync = config->getBools("engine", "worldStepAsync")[0];
	stepThreadCount = config->getInts("engine", "worldStepThreads")[0];

	debugf("World init: steps: %d, async: %d, threads: %d", stepsPerSecondTarget, stepsAsync, stepThreadCount);

	lastStep = Time::now();
	deltaTime = 1000.0 / stepsPerSecondTarget;

	class SomeComp : public Component<SomeComp>
	{
	public:
		int someInt = 10;
	};

	class SomeSystem : public System<SomeComp>
	{
	public:

	};

	auto system = ECS::makeSystem<SomeComp>();
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

	if (stepsAsync)
	{
		this->start();
	}
	else
	{
		V3::getInstance()->addTickable(getInstance());
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
		for (auto kv : ECS::systems)
		{
			(&kv)->second->tick_func();
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
	}
}