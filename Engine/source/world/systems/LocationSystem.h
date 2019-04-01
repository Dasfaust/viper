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
			auto entity = container->getEntity(comp->entity);
			auto& type = container->resolveType<MovementInputComponent>();
			if (entity->components.count(type.id))
			{
				MovementInputComponent* m = container->getComponent<MovementInputComponent>(entity);
				comp->location.x += m->forward;
				comp->location.z += m->right;
			}

			//debugf("Location: %.0f, %.0f, %.0f", comp->location.x, comp->location.y, comp->location.z);
		});

		setWaitFunction([](ECS::System* system, World* world)
		{
			
		});
	};
private:
};