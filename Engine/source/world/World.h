#pragma once
#include "../Module.h"
#include "../Threadable.h"
#include "../ecs/ECS.h"
#include "concurrentqueue.h"
#include <future>
#include "../util/Time.h"
#include "tbb/concurrent_vector.h"
#include <atomic>
#include <glm/vec2.hpp>
#include "../util/Any.h"
#include "../util/Profiler.h"
#include "../networking/Networking.h"
#include <boost/functional/hash.hpp>

using timeStep = std::chrono::duration<float, std::ratio<1, 30>>;

class Worker;

struct MapCell
{
	uint32 entity;
	glm::vec2 position;
	glm::vec2 positionLast;
};

struct Player
{
	NetworkPlayer netPlayer;
	glm::vec3 pos;
};

class World : public Module, Threadable
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

		std::vector<std::shared_ptr<ECS::TypeInfo>> components;
		void(*callback)(uint32, ECS::Component*, std::vector<Any>);
		std::vector<Any> callbackVars;

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
			while (!queue.try_dequeue(obj)) {}
			return obj;
		};

		inline void fulfill(T obj)
		{
			queue.enqueue(obj);
		};
	private:
		moodycamel::ConcurrentQueue<T> queue;
	};

	const std::vector<std::string> headings = { "E", "NE", "N", "NW", "W", "SW", "S", "SE" };

	std::vector<std::shared_ptr<Worker>> workers;
	tbb::concurrent_vector<std::shared_ptr<moodycamel::ConcurrentQueue<bool>>> queues;
	moodycamel::ConcurrentQueue<std::pair<uint32, std::vector<std::shared_ptr<ECS::TypeInfo>>>> systemsToCreate;
	moodycamel::ConcurrentQueue<std::shared_ptr<Future<ECS::Entity*>>> entsToCreate;
	moodycamel::ConcurrentQueue<ECS::Entity*> entsToDelete;
	moodycamel::ConcurrentQueue<ECS::Component*> compsToDelete;
	boost::container::flat_map<uint8, moodycamel::ConcurrentQueue<ECS::Changeset*>> changesetQueue;
	ECS::Container* ecs;
	std::atomic<bool> paused = false;
	moodycamel::ConcurrentQueue<MapCell> mapGridUpdates;
	std::unordered_map<int, std::vector<MapCell>> map;
	int mapHeight = 9;
	int mapWidth = 9;
	std::atomic<bool> loaded = false;
	std::atomic<float> loadProgress = 0.0f;
	boost::container::flat_map<boost::uuids::uuid, Player> players;
	boost::container::flat_map<unsigned int, std::shared_ptr<Pool>> changesets;


	V3API World();
	V3API ~World();

	V3API void onStartup() override;
	V3API void tick() override;
	V3API void onShutdown() override;

	template<typename T, typename A>
	inline void createSystem(uint32 index)
	{
		std::vector<std::shared_ptr<ECS::TypeInfo>> vec{ ecs->resolveType<T>(), ecs->resolveType<A>() };
		debugf("System creation queued: %s -> %s", typeid(T).name(), typeid(A).name());
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
	inline std::shared_ptr<Future<ECS::Entity*>> createEntity(void(*callback)(uint32, ECS::Component*, std::vector<Any>) = nullptr, std::vector<Any> vars = { })
	{
		auto future = createEntity();
		future->callback = callback;
		future->callbackVars = vars;
		future->components = std::vector<std::shared_ptr<ECS::TypeInfo>>{ ecs->resolveType<A>() };
		//debugf("Component creation queued: %s", typeid(A).name());
		return future;
	};
	template<typename A, typename B>
	inline std::shared_ptr<Future<ECS::Entity*>> createEntity(void(*callback)(uint32, ECS::Component*, std::vector<Any>) = nullptr, std::vector<Any> vars = { })
	{
		auto future = createEntity();
		future->callback = callback;
		future->callbackVars = vars;
		future->components = std::vector<std::shared_ptr<ECS::TypeInfo>>{ ecs->resolveType<A>(), ecs->resolveType<B>() };
		//debugf("Component creation queued: %s, %s", typeid(A).name(), typeid(B).name());
		return future;
	};
	template<typename A, typename B, typename C>
	inline std::shared_ptr<Future<ECS::Entity*>> createEntity(void(*callback)(uint32, ECS::Component*, std::vector<Any>) = nullptr, std::vector<Any> vars = { })
	{
		auto future = createEntity();
		future->callback = callback;
		future->callbackVars = vars;
		future->components = std::vector<std::shared_ptr<ECS::TypeInfo>>{ ecs->resolveType<A>(), ecs->resolveType<B>(), ecs->resolveType<C>() };
		//debugf("Component creation queued: %s, %s", typeid(A).name(), typeid(B).name(), typeid(C).name());
		return future;
	};
	template<typename A, typename B, typename C, typename D>
	inline std::shared_ptr<Future<ECS::Entity*>> createEntity(void(*callback)(uint32, ECS::Component*, std::vector<Any>) = nullptr, std::vector<Any> vars = { })
	{
		auto future = createEntity();
		future->callback = callback;
		future->callbackVars = vars;
		future->components = std::vector<std::shared_ptr<ECS::TypeInfo>>{ ecs->resolveType<A>(), ecs->resolveType<B>(), ecs->resolveType<C>(), ecs->resolveType<D>() };
		//debugf("Component creation queued: %s, %s", typeid(A).name(), typeid(B).name(), typeid(C).name(), typeid(D).name());
		return future;
	};
	template<typename A, typename B, typename C, typename D, typename E>
	inline std::shared_ptr<Future<ECS::Entity*>> createEntity(void(*callback)(uint32, ECS::Component*, std::vector<Any>) = nullptr, std::vector<Any> vars = { })
	{
		auto future = createEntity();
		future->callback = callback;
		future->callbackVars = vars;
		future->components = std::vector<std::shared_ptr<ECS::TypeInfo>>{ ecs->resolveType<A>(), ecs->resolveType<B>(), ecs->resolveType<C>(), ecs->resolveType<D>() , ecs->resolveType<E>() };
		//debugf("Component creation queued: %s, %s", typeid(A).name(), typeid(B).name(), typeid(C).name(), typeid(D).name(), typeid(E).name());
		return future;
	};

	V3API void tickSystem(ECS::System* system, uint8 type, unsigned int worker, int start = 0, int end = -1);

	std::shared_ptr<ECS::TypeInfo> loc_t;
	std::shared_ptr<ECS::TypeInfo> bb_t;
	V3API std::unordered_map<std::string, std::vector<MapCell>> getNearbyEntities2D(ECS::Entity* ent, unsigned int radius);
	V3API std::string getDirection2D(glm::vec2 pos1, glm::vec2 pos2);

	V3API void queueMapUpdate(glm::vec2 now, glm::vec2 last, unsigned int entity);

	inline void queueChangeset(std::shared_ptr<ECS::TypeInfo> type, ECS::Changeset* change)
	{
		changesetQueue[type->id].enqueue(change);
	};

	template<typename T>
	inline void registerChangeset()
	{
		for (unsigned int i = 1; i < stepThreadCount; i++)
		{
			changesets[i]->resolve<T>();
		}
	};

	template<typename T>
	inline T* makeChangeset(unsigned int worker, unsigned int component)
	{
		auto change = changesets[worker]->create<T>();
		change->worker = worker;
		change->index = component;
		return change;
	};

	struct Component : public object
	{
		unsigned int entity;
	};

	class System
	{
	public:
		std::type_index componentType;
	private:
	};

	inline float getDistance2D(glm::vec2 first, glm::vec2 second)
	{
		return sqrt(pow(second.x - first.x, 2) + pow(second.y - first.y, 2) * 1.0f);
	};

	template<typename T>
	inline T* getComponent()
	{
		
	};
private:
    int stepThreadCount = 2;
	double targetDeltaTime = 0.0;
	double actualDeltaTime = 0.0;
	double actualStepDelta = 0.0;
	double lastTickEnd = 0.0;
	double lastStepEnd = 0.0;
	double perfAccumulator = 0.0;
	double stepAccumulator = 0.0;
	bool firstTick = true;
	double nextStep = 0.0;

	std::shared_ptr<Pool> pool;
	std::unordered_map<unsigned int, std::unordered_map<unsigned int, unsigned int>> entities;

	inline unsigned int makeEntity()
	{
		return entities.size();
	};

	inline unsigned int makeComponent(unsigned int entity, std::type_index typeIndex)
	{
		auto type = pool->getType(typeIndex);
		auto comp = reinterpret_cast<Component*>(pool->create(type));
		entities[entity][type->id] = comp->id;
		comp->entity = entity;
		return comp->id;
	};

	inline void delComponent(unsigned int entity, unsigned int component, std::type_index typeIndex)
	{
		auto type = pool->getType(typeIndex);
		pool->del(type, component);
		entities[entity].erase(type->id);
	};

	inline void delEntity(unsigned int entity)
	{
		for (auto&& kv : entities[entity])
		{
			auto type = pool->getType(kv.first);
			pool->del(type, kv.second);
		}
		entities.erase(entity);
	};
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
	bool hasJob = false;
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
		if (world->queues.size() >= id)
		{
			bool ready = false;
			while (!(world->queues[id]->try_dequeue(ready)))
			{
				if (!running) break;
				// Adds ~2ms to step time but CPU savings... Is there another way?
				std::this_thread::sleep_for(std::chrono::microseconds(50));
			}

			if (ready)
			{
				Job job;
				double jobStart = tnow();
				while (jobs.try_dequeue(job))
				{
					//profiler_begin("component_" + std::to_string(job.type), world);
					//debugf("Worker %d tick...", id);
					double start = tnow();
					world->tickSystem(job.system, job.type, id, job.start, job.end);
					//debugf("Job tickSystem for type %d took %.2f ms", job.type, tnow() - start);
				}
				finished.enqueue(true);
				//debugf("Worker %d jobs completed in %.2f ms", id, tnow() - jobStart);
			}
		}
	};
};