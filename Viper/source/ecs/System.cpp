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

			for (int sys : ent->systems)
			{
				if (sys > -1)
				{
					container->systems[sys]->updateEntity(ent, container->systems[sys], (float)(container->dt / 1000.0));
				}
			}
		}

		/*for (uint64 i = currentJob.start * container->entitySize; i <= currentJob.end * container->entitySize; i += container->entitySize)
		{
			auto ent = reinterpret_cast<ecs::Entity*>(&container->heap[i]);

			if (ent->skip)
			{
				continue;
			}

			for (uint32 i = 0; i < container->systemTypes.size(); i++)
			{
				std::set<uint32> types = container->systemTypes[i];

				bool update = true;
				for (uint32 type : types)
				{
					if (ent->componentPointers[type] == nullptr)
					{
						update = false;
						break;
					}
				}
				if (update)
				{
					//while(container->getFlag(ent->id)) {}
					container->systems[i]->updateEntity(ent, container->systems[i], (float)(container->dt / 1000.0));
				}
			}
		}*/

		completed.enqueue(currentJob);
		currentJob.valid = false;
	}

	jobs.try_dequeue(currentJob);
};
