#pragma once
#include "../Module.h"
#include "../Threadable.h"
#include "../ecs/ECS.h"
#include <unordered_map>
#include <tuple>
#include "concurrentqueue.h"
#include <future>

class Worker;

class World : public Module, public Threadable
{
public:
	template<typename T>
	struct PromiseState
	{
		bool finished;
		T returned;
	};

	template<typename T>
	class Future
	{
	public:
		static std::shared_ptr<Future<T>> create()
		{
			return std::make_shared<Future<T>>();
		};

		std::vector<ECS::TypeInfo> components;
		void(*callback)(uint32, ECS::Component*);

		inline PromiseState<T> tryGet()
		{
			T obj;
			if (queue.try_dequeue(obj))
			{
				return { true, obj };
			}
			else
			{
				return { false };
			}
		};

		inline T get()
		{
			T obj;
			while (!queue.try_dequeue(obj)) { }
			return obj;
		};

		inline void fulfill(T obj)
		{
			queue.enqueue(obj);
		};
	private:
		moodycamel::ConcurrentQueue<T> queue;
	};

	double stepAlpha = 0.0;
	int stepsPerSecond = 0;
	float stepTarget = 0.0;
	std::vector<std::shared_ptr<Worker>> workers;
	std::vector<std::shared_ptr<moodycamel::ConcurrentQueue<bool>>> queues;
	moodycamel::ConcurrentQueue<std::pair<uint32, std::vector<ECS::TypeInfo>>> systemsToCreate;
	moodycamel::ConcurrentQueue<std::shared_ptr<Future<ECS::Entity*>>> entsToCreate;
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

	template<typename T, typename A>
	inline void createSystem(uint32 index)
	{
		std::vector<ECS::TypeInfo> vec{ ecs->resolveType<T>(), ecs->resolveType<A>() };
		debugf("System creation queued: %s -> %s", typeid(T).name(), typeid(A).name());
		systemsToCreate.enqueue(std::make_pair(index, vec));
	};

	template<typename T, typename A, typename B>
	inline void createSystem(uint32 index)
	{
		std::vector<ECS::TypeInfo> vec{ ecs->resolveType<T>(), ecs->resolveType<A>(),  ecs->resolveType<B>() };
		debugf("System creation queued: %s -> %s, %s", typeid(T).name(), typeid(A).name(), typeid(B).name());
		systemsToCreate.enqueue(std::make_pair(index, vec));
	};

	template<typename T, typename A, typename B, typename C>
	inline void createSystem(uint32 index)
	{
		std::vector<ECS::TypeInfo> vec{ ecs->resolveType<T>(), ecs->resolveType<A>(),  ecs->resolveType<B>(), ecs->resolveType<C>() };
		debugf("System creation queued: %s -> %s, %s, %s", typeid(T).name(), typeid(A).name(), typeid(B).name(), typeid(C).name());
		systemsToCreate.enqueue(std::make_pair(index, vec));
	};

	template<typename T, typename A, typename B, typename C, typename E>
	inline void createSystem(uint32 index)
	{
		std::vector<ECS::TypeInfo> vec{ ecs->resolveType<T>(), ecs->resolveType<A>(),  ecs->resolveType<B>(), ecs->resolveType<C>(), ecs->resolveType<E>() };
		debugf("System creation queued: %s -> %s, %s, %s, %s", typeid(T).name(), typeid(A).name(), typeid(B).name(), typeid(C).name(), typeid(E).name());
		systemsToCreate.enqueue(std::make_pair(index, vec));
	};

	template<typename T, typename A, typename B, typename C, typename E, typename F>
	inline void createSystem(uint32 index)
	{
		std::vector<ECS::TypeInfo> vec{ ecs->resolveType<T>(), ecs->resolveType<A>(),  ecs->resolveType<B>(), ecs->resolveType<C>(), ecs->resolveType<E>(), ecs->resolveType<F>() };
		debugf("System creation queued: %s -> %s, %s, %s, %s, %s", typeid(T).name(), typeid(A).name(), typeid(B).name(), typeid(C).name(), typeid(E).name(), typeid(F).name());
		systemsToCreate.enqueue(std::make_pair(index, vec));
	};

	inline void deleteEntity(ECS::Entity* entity)
	{
		entsToDelete.enqueue(entity);
	};

	inline void deleteComponent(ECS::Component* component)
	{
		compsToDelete.enqueue(component);
	};

	inline std::shared_ptr<Future<ECS::Entity*>> createEntity()
	{
		auto future = Future<ECS::Entity*>::create();
		entsToCreate.enqueue(future);
		return future;
	};
	template<typename A>
	inline std::shared_ptr<Future<ECS::Entity*>> createEntity(void(*callback)(uint32, ECS::Component*) = nullptr)
	{
		auto future = createEntity();
		future->callback = callback;
		future->components = std::vector<ECS::TypeInfo>{ ecs->resolveType<A>() };
		debugf("Component creation queued: %s", typeid(A).name());
		return future;
	};
	template<typename A, typename B>
	inline std::shared_ptr<Future<ECS::Entity*>> createEntity(void(*callback)(uint32, ECS::Component*) = nullptr)
	{
		auto future = createEntity();
		future->callback = callback;
		future->components = std::vector<ECS::TypeInfo>{ ecs->resolveType<A>(), ecs->resolveType<B>() };
		debugf("Component creation queued: %s, %s", typeid(A).name(), typeid(B).name());
		return future;
	};
	template<typename A, typename B, typename C>
	inline std::shared_ptr<Future<ECS::Entity*>> createEntity(void(*callback)(uint32, ECS::Component*) = nullptr)
	{
		auto future = createEntity();
		future->callback = callback;
		future->components = std::vector<ECS::TypeInfo>{ ecs->resolveType<A>(), ecs->resolveType<B>(), ecs->resolveType<C>() };
		debugf("Component creation queued: %s, %s", typeid(A).name(), typeid(B).name(), typeid(C).name());
		return future;
	};
	template<typename A, typename B, typename C, typename D>
	inline std::shared_ptr<Future<ECS::Entity*>> createEntity(void(*callback)(uint32, ECS::Component*) = nullptr)
	{
		auto future = createEntity();
		future->callback = callback;
		future->components = std::vector<ECS::TypeInfo>{ ecs->resolveType<A>(), ecs->resolveType<B>(), ecs->resolveType<C>(), ecs->resolveType<D>() };
		debugf("Component creation queued: %s, %s", typeid(A).name(), typeid(B).name(), typeid(C).name(), typeid(D).name());
		return future;
	};
	template<typename A, typename B, typename C, typename D, typename E>
	inline std::shared_ptr<Future<ECS::Entity*>> createEntity(void(*callback)(uint32, ECS::Component*) = nullptr)
	{
		auto future = createEntity();
		future->callback = callback;
		future->components = std::vector<ECS::TypeInfo>{ ecs->resolveType<A>(), ecs->resolveType<B>(), ecs->resolveType<C>(), ecs->resolveType<D>() , ecs->resolveType<E>() };
		debugf("Component creation queued: %s, %s", typeid(A).name(), typeid(B).name(), typeid(C).name(), typeid(D).name(), typeid(E).name());
		return future;
	};

	V3API void tickSystem(ECS::System* system, uint8 type, int start = 0, int end = -1);
private:
    int stepsPerSecondTarget;
    bool stepsAsync = false;
    int stepThreadCount = 1;

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