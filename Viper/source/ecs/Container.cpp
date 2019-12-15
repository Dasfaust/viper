#include "Container.hpp"
#include "../log/Logger.hpp"
#include "../interface/Threadable.hpp"

using namespace ecs;

void Container::onStart()
{
	entitySize = sizeof(Entity);
	size_t offset = entitySize;
	for (auto meta : componentData)
	{
		if (offsets.size() < meta.id + 1) offsets.resize(meta.id + 1);
		offsets[meta.id] = offset;

		entitySize += meta.tSize;
		offset += meta.tSize;
	}
	debug("Entity size: %d (total: %d)", sizeof(Entity), entitySize);
	for (auto& meta : componentData)
	{
		debug("Component %d: size: %d, tsize: %d offset: %d, flag offset: %d", meta.id, meta.size, meta.tSize, offsets[meta.id], offsets[meta.id] + meta.size);
	}

	if (async)
	{
		for (uint32 i = 0; i < threads; i++)
		{
			auto mod = initModule<Worker>(std::to_string(i));
			mod->id = i;
			mod->sleep = false;
			info("Starting ECS worker %d...", i);
			mod->start();
		}
	}
};

void Container::onShutdown()
{
	for (auto&& kv : modules)
	{
		auto mod = std::dynamic_pointer_cast<Threadable>(kv.second);
		mod->stop();
	}
	
	purge();
};

void Container::purge()
{
	heap.clear();
	heap.shrink_to_fit();
};

void Container::allocateNewBlock()
{
	info("Reallocating ECS heap");
	heap.reserve(heap.size() + (blockSizeMb * 1024000 / sizeof(uint32)));
	blocksAllocated++;
}

uint64 Container::getNextEntityId()
{
	return deletions.empty() ? (uint64)(heap.size() / entitySize) : [this]() -> uint64 { uint64 free = deletions[0]; deletions.erase(deletions.begin()); return free; }();
};

uint64 Container::makeEntity(std::set<uint32> comps)
{
	uint64 id = getNextEntityId();
	if (blocksAllocated * blockSizeMb * 1024000 < heap.size() * sizeof(uint32)) allocateNewBlock();
	if (id >= heap.size() / entitySize) heap.resize(heap.size() + entitySize);
	
	uint64 index = id * (uint64)entitySize;
	Entity* ent = new(&heap[index]) Entity();
	ent->id = id;

	for (uint32 i = 0; i < ent->components.size(); i++)
	{
		ent->components[i] = false;
	}

	size_t offset = sizeof(Entity);
	for (auto meta : componentData)
	{
		auto ptr = meta.instantiate(heap, (uint64)(index + offset), meta);
		if (comps.find(meta.id) != comps.end())
		{
			ent->components[meta.id] = true;
		}
		offset += meta.tSize;
	}

	updateSystemsCache(id);

	return id;
};

void Container::updateSystemsCache(uint64 id)
{
	auto ent = getEntity(id);

	for (uint32 i = 0; i < ent->systems.size(); i++)
	{
		ent->systems[i] = false;
	}
	
	for (uint32 i = 0; i < systemTypes.size(); i++)
	{
		auto types = systemTypes[i];

		bool update = true;
		bool foundAny = false;
		uint32 index = 0;
		for (auto type : types)
		{
			index++;

			if (!ent->components[type.first])
			{
				if (type.second == ecs::ComponentFlags::E_OPTIONAL || type.second == ecs::ComponentFlags::E_SKIP)
				{
					if (index == types.size() && !foundAny)
					{
						update = false;
					}

					continue;
				}

				update = false;
				break;
			}

			if (type.second == ecs::ComponentFlags::E_SKIP)
			{
				update = false;
			}

			foundAny = true;
		}
		if (update)
		{
			ent->systems[i] = true;
		}
	}
};

Entity* Container::getEntity(uint64 id)
{
	return reinterpret_cast<Entity*>(&heap[id * entitySize]);
};

void Container::deleteEntity(uint64 id)
{
	getEntity(id)->skip = true;
	deletions.push_back(id);
};

void Container::onTick()
{
	if (firstTick)
	{
		for (auto& sys : systems)
		{
			sys->onStart();
		}

		firstTick = false;
	}

	tickModules();

	for (auto& sys : systems)
	{
		sys->onTickBegin();
	}

	if (async)
	{
		uint32 entities = (uint32)(heap.size() / entitySize);
		uint32 entsPerWorker = entities / threads;
		uint32 lastIndex = 0;
		uint32 threadIndex = 0;
		
		std::vector<moodycamel::ConcurrentQueue<Job>*> results;
		for (auto&& kv : modules)
		{
			if (entities <= 1)
			{
				auto worker = std::static_pointer_cast<Worker>(kv.second);
				worker->jobs.enqueue({ lastIndex, 0, true });
				results.push_back(&worker->completed);
				break;
			}

			auto worker = std::static_pointer_cast<Worker>(kv.second);
			uint64 end = threadIndex + 1 >= threads ? entities - 1 : lastIndex + entsPerWorker - 1;
			worker->jobs.enqueue({ lastIndex, end, true });
			lastIndex += entsPerWorker;
			threadIndex++;

			results.push_back(&worker->completed);
			
		}
		for (auto queue : results)
		{
			Job job;
			while (!queue->try_dequeue(job)) {};
		}
		results.clear();
	}
	else
	{
		for (uint64 i = 0; i < heap.size(); i += entitySize)
		{
			auto ent = reinterpret_cast<Entity*>(&heap[i]);

			if (ent->skip) { continue; }
			
			uint32 updated = 0;
			uint32 index = 0;
			while(updated < ent->systems.count())
			{
				if (ent->systems[index])
				{
					systems[index]->updateEntity(ent, systems[index], Time::toSeconds(deltaTime));
					updated++;
				}
				index++;
			}
		}
	}

	for (auto& sys : systems)
	{
		sys->onTickEnd();
	}
};