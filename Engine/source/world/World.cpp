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