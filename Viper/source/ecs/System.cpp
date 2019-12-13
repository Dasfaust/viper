#include "System.hpp"
#include "Container.hpp"

using namespace ecs;

void Worker::onTickAsync()
{
	if (currentJob.valid)
	{
		auto container = getParent<Container>();

		for (uint64 i = currentJob.start * container->entitySize; i <= currentJob.end * container->entitySize; i += container->entitySize)
		{
			auto ent = reinterpret_cast<Entity*>(&container->heap[i]);

			if (ent->skip) { continue; }

			uint32 updated = 0;
			uint32 index = 0;
			while (updated < ent->systems.count())
			{
				if (ent->systems[index])
				{
					container->systems[index]->updateEntity(ent, container->systems[index], (float)(container->deltaTimeMs / 1000.0f));
					updated++;
				}
				index++;
			}
		}

		completed.enqueue(currentJob);
		currentJob.valid = false;
	}

	jobs.try_dequeue(currentJob);
};
