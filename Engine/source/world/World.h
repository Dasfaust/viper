#pragma once
#include "../EngineExtension.h"
#include "../Threadable.h"
#include "ECS.h"
#include <unordered_map>

#define EXT_WORLD_ADD() V3::getInstance()->addExtension(World::getInstance(), 0.0);

class World : public EngineExtension, public Threadable
{
public:
	static V3API std::shared_ptr<World> getInstance();

	~World();

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
protected:
    static std::shared_ptr<World> instance;
    World();
private:
    int stepsPerSecondTarget;
    bool stepsAsync = false;
    int stepThreadCount = 1;

    int stepsPerSecond;
    double stepAlpha;
    int stepCount;
    double lastStep;
    double stepTime;
    double deltaTime;
    double stepAccumulator;
    double stepPerformanceAccumulator;
};

struct WorldContainer : public World
{
	WorldContainer() : World() { }
};