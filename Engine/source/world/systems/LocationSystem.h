#pragma once
#include "Components.h"
#include "../World.h"
#include "../../V3.h"

class LocationSystem : public ECS::System
{
public:
	inline void init(ECS::Container* container, World* world) override
	{
		

		//debug("Location init");
		setTickFunction([](double dt, ECS::Component* component, ECS::System* system, ECS::Container* container, World* world)
		{
			LocationComponent* comp = reinterpret_cast<LocationComponent*>(component);

			glm::vec3 loc = comp->location;

			/*ECS::Changeset change = { comp->index, 0, loc };
			world->changesets[comp->type_id].enqueue(change);*/

			MapCell cell = { container->getEntity(comp->entity), glm::vec2(comp->location.x, comp->location.y) };
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