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

			if (!ent->skip)
			{
				for (uint32 id : ent->components)
				{
					if (container->systems.size() > id)
					{
						auto system = container->systems[id];
						if (system != nullptr)
						{
							auto flag = container->getFlag(ent->id, id);
							while (flag->test_and_set()) {};
							// TODO IMPORTANT DT
							system->updateEntity(ent, container->getComponent(ent->id, id), system, 0.0);
							flag->clear();
						}
					}
				}

				currentJob.iterated++;
			}
		}

		completed.enqueue(currentJob);
		currentJob.valid = false;
	}

	jobs.try_dequeue(currentJob);
};
