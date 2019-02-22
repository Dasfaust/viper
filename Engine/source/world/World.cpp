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

}

World::~World()
{

	for (std::unordered_map<unsigned int, std::vector<unsigned int>>::iterator it = components.begin(); it != components.end(); it++)
	{
		size_t size = ComponentBase::getSize(it->first);
		auto removeFn = ComponentBase::getRemoveFn(it->first);
		for (int i = 0; i < it->second.size(); i += size)
		{
			removeFn((ComponentBase*)&it->second[i]);
		}
	}

	for (unsigned int e = 0; e < entities.size(); e++)
	{
		delete entities[e];
	}
}

void World::onStartup()
{
    auto config = V3::getInstance()->getConfig();
    stepsPerSecondTarget = config->getInts("engine", "worldStepsPerSecond")[0];
    stepsAsync = config->getBools("engine", "worldStepAsync")[0];
    stepThreadCount = config->getInts("engine", "worldStepThreads")[0];

    debugf("World init: steps: %d, async: %d, threads: %d", stepsPerSecondTarget, stepsAsync, stepThreadCount);

    if (stepsAsync)
    {
        this->start();
    }
    else
    {
        V3::getInstance()->addTickable(getInstance());
    }

    lastStep = Time::now();
    deltaTime = 1000.0 / stepsPerSecondTarget;
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

        stepAccumulator -= deltaTime;
        stepCount++;
    }

    stepAlpha = stepAccumulator / deltaTime;
}

EntityHandle World::makeEntity(ComponentBase* components, const unsigned int* componentTypes, size_t num)
{
	auto ent = new std::pair<unsigned int, std::vector<std::pair<unsigned int, unsigned int>>>();
	auto handle = (EntityHandle)ent;

	for (unsigned int i = 0; i < num; i++)
	{
		auto createFn = ComponentBase::getCreateFn(componentTypes[i]);

		auto pair = std::pair<unsigned int, unsigned int>();
		pair.first = componentTypes[i];
		pair.second = createFn(this->components[componentTypes[i]], handle, &components[i]);

		ent->second.push_back(pair);
	}

	unsigned int id = entities.size();
	entities.push_back(ent);

	return handle;
}

void World::removeComponentInternal(unsigned int id, unsigned int index)
{

}

void World::removeEntity(EntityHandle handle)
{
	auto ent = getEntityRaw(handle);
	for (int i = 0; i < ent.size(); i++)
	{
		removeComponentInternal(ent[i].first, ent[i].second);
	}

	unsigned int dest = getEntityIndex(handle);
	unsigned int last = entities.size() - 1;
	delete entities[dest];
	entities[dest] = entities[last];
	entities.pop_back();
}