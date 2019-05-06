#pragma once
#include "Components.h"
#include "../World.h"
#include "../../V3.h"

class LocationSystem : public ECS::System
{
public:
	inline void init(ECS::Container* container, World* world) override
	{
		setTickFunction([](double dt, ECS::Component* component, ECS::System* system, ECS::Container* container, World* world)
		{
			LocationComponent* comp = reinterpret_cast<LocationComponent*>(component);

			glm::vec3 loc = comp->location;

			// TODO: do this better
			glm::vec2 last(0.0f, 0.0f);
			for (auto&& kv : world->map)
			{
				for (auto c : kv.second)
				{
					if (c.entity != 0 && c.entity->index == comp->entity)
					{
						last = c.position;
					}
				}
			}
			MapCell cell = { container->getEntity(comp->entity), glm::vec2(comp->location.x, comp->location.z), last };
			world->mapGridUpdates.enqueue(cell);
		});

		setApplyChangeFunction([](std::string name, ECS::Component* comp, ECS::Changeset change)
		{
			if (name == "LocationComponent")
			{
				auto c = reinterpret_cast<LocationComponent*>(comp);
				if (change.field == 0)
				{
					c->location = boost::any_cast<glm::vec3>(change.value);
				}
			}
		});

		setWaitFunction([](ECS::System* system, World* world)
		{
			
		});
	};
private:
};