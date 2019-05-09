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
		{ });

		setApplyChangeFunction([](std::string name, ECS::Component* comp, ECS::Changeset change, World* world)
		{
			if (name == "LocationComponent")
			{
				auto c = reinterpret_cast<LocationComponent*>(comp);
				if (change.field == 0)
				{
					glm::vec3 loc = boost::any_cast<glm::vec3>(change.value);
					world->queueMapUpdate(glm::vec2(loc.x, loc.z), glm::vec2(c->location.x, c->location.z), comp->entity);
					c->location = loc;
				}
			}
		});

		setWaitFunction([](ECS::System* system, World* world)
		{
			
		});
	};
private:
};