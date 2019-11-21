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
	debug("Entity size: %d", entitySize);
	for (auto& meta : componentData)
	{
		debug("Comp %d: size: %d, tsize: %d offset: %d, moffset: %d", meta.id, meta.size, meta.tSize, offsets[meta.id], offsets[meta.id] + meta.size);
	}

	if (async)
	{
		for (uint32 i = 0; i < threads; i++)
		{
			auto mod = initModule<Worker>(std::to_string(i));
			mod->id = i;
			mod->sleep = false;
			debug("Starting ECS worker %d...", i);
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
	debug("Reallocating ECS heap");
	heap.reserve(heap.size() + (blockSizeMb * 1024000 / sizeof(uint32)));
	blocksAllocated++;
}

uint64 Container::getNextEntityId()
{
	return deletions.empty() ? (uint64)(heap.size() / entitySize) : [this]() -> uint64 { uint64 free = deletions[0]; deletions.erase(deletions.begin()); return free; }();
};

uint64 Container::makeEntity(std::vector<uint32> comps)
{
	uint64 id = getNextEntityId();
	if (blocksAllocated * blockSizeMb * 1024000 < heap.size() * sizeof(uint32)) allocateNewBlock();
	if (id >= heap.size() / entitySize) heap.resize(heap.size() + entitySize);
	uint64 index = id * (uint64)entitySize;
	Entity* ent = new(&heap[index]) Entity();
	ent->components = comps;
	ent->id = id;
	size_t offset = sizeof(Entity);
	for (auto meta : componentData)
	{
		meta.instantiate(heap, (uint64)(index + offset), meta);
		offset += meta.tSize;
	}
	return id;
};

ecs::Entity* Container::getEntity(uint64 id)
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
	if (async)
	{
		uint32 entsPerWorker = (heap.size() / entitySize) / threads;
		uint32 lastIndex = 0;
		uint32 threadIndex = 0;
		for (auto&& kv : modules)
		{
			auto worker = std::static_pointer_cast<Worker>(kv.second);
			uint64 end = threadIndex + 1 >= threads ? (heap.size() / entitySize) - 1 : lastIndex + entsPerWorker - 1;
			worker->jobs.enqueue({ lastIndex, end, true });
			lastIndex += entsPerWorker;
			threadIndex++;
		}
		for (auto&& kv : modules)
		{
			auto worker = std::static_pointer_cast<Worker>(kv.second);
			Job job;
			while (!worker->completed.try_dequeue(job)) {};
		}
	}
	else
	{
		for (uint64 i = 0; i < heap.size(); i += entitySize)
		{
			auto ent = reinterpret_cast<ecs::Entity*>(&heap[i]);

			for (uint32 id : ent->components)
			{
				if (systems.size() > id)
				{
					auto system = systems[id];
					if (system != nullptr)
					{
						system->updateEntity(ent, getComponent(ent->id, id), system);
					}
				}
			}
		}
	}
};