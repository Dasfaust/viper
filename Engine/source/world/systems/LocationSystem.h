#pragma once
#include "Components.h"
#include "../World.h"
#include "../../V3.h"

class LocationSystem : public ECS::System
{
public:
	std::shared_ptr<ECS::TypeInfo> rend_t;

	inline void init(ECS::Container* container, World* world) override
	{
		rend_t = container->resolveType<RenderComponent>();

		setTickFunction([](double dt, ECS::Component* component, ECS::System* system, ECS::Container* container, World* world)
		{ });

		setApplyChangeFunction([](std::string name, ECS::Component* comp, ECS::Changeset change, World* world, System* system)
		{
			auto c = reinterpret_cast<LocationComponent*>(comp);
			if (change.field == 0)
			{
				glm::vec3 loc = any::vec3(change.value);
				world->queueMapUpdate(glm::vec2(loc.x, loc.z), glm::vec2(c->location.x, c->location.z), comp->entity);
				c->location = loc;

				ECS::Entity* ent = world->ecs->getEntity(comp->entity);
				LocationSystem* sys = reinterpret_cast<LocationSystem*>(system);
				if (ent->components.count(sys->rend_t->id))
				{
					for (auto&& kv : world->players)
					{
						RenderComponent* rend = reinterpret_cast<RenderComponent*>(world->ecs->getComponent(ent, sys->rend_t->id));
						Any a = any::make(true);
						a.uuid_val = kv.first;
						world->queueChangeset(sys->rend_t, { rend->index, 0, a });
					}
				}
			}
		});

		setWaitFunction([](ECS::System* system, World* world)
		{
			
		});
	};
private:
};