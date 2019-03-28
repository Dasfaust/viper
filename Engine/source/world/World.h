#pragma once
#include "../Module.h"
#include "../Threadable.h"
#include "../ecs/ECS.h"
#include <unordered_map>
#include <tuple>
#include "concurrentqueue.h"

class Worker;

class World : public Module, public Threadable
{
public:
	int stepsPerSecond = 0;
	std::vector<std::shared_ptr<Worker>> workers;
	std::vector<std::shared_ptr<moodycamel::ConcurrentQueue<bool>>> queues;
	moodycamel::ConcurrentQueue<ECS::Entity*> entsToDelete;
	moodycamel::ConcurrentQueue<ECS::Component*> compsToDelete;
	ECS::Container* ecs;

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

	inline void deleteEntity(ECS::Entity* entity)
	{
		entsToDelete.enqueue(entity);
	};

	inline void deleteComponent(ECS::Component* component)
	{
		compsToDelete.enqueue(component);
	};

	V3API void tickSystem(ECS::System* system, uint8 type, int start = -1, int end = -1);
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
	struct Job
	{
		ECS::System* system;
		uint8 type;
		int start;
		int end;
	};

	unsigned int id;
	moodycamel::ConcurrentQueue<Job> jobs;
	moodycamel::ConcurrentQueue<bool> finished;
	World* world;

	Worker(World* world, unsigned int id)
	{
		this->world = world;
		this->id = id;
	};

	inline void queueJob(Job job)
	{
		jobs.enqueue(job);
	};

	void tick() override
	{
		bool ready;
		if (world->queues[id]->try_dequeue(ready))
		{
			Job job;
			while (jobs.try_dequeue(job))
			{
				//debugf("Worker %d tick...", id);
				world->tickSystem(job.system, job.type, job.start, job.end);
			}
			finished.enqueue(true);
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	};
};