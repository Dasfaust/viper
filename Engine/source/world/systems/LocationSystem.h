#pragma once
#include "Components.h"
#include "../World.h"
#include "../../V3.h"
#include "RenderSystem.h"

struct LocationChangeset : public ECS::Changeset
{
	glm::vec3 location;
};

class LocationSystem : public ECS::System
{
public:
	std::shared_ptr<ECS::TypeInfo> rend_t;

	inline void init(ECS::Container* container, World* world) override
	{
		world->registerChangeset<LocationChangeset>();

		rend_t = container->resolveType<RenderComponent>();

		setTickFunction([](double dt, ECS::Component* component, ECS::System* system, ECS::Container* container, World* world, unsigned int worker)
		{ });

		setApplyChangeFunction([](std::string name, ECS::Component* comp, ECS::Changeset* change, World* world, System* system)
		{
			auto c = reinterpret_cast<LocationComponent*>(comp);
			if (change->field == 0)
			{
				glm::vec3 loc = reinterpret_cast<LocationChangeset*>(change)->location;
				world->queueMapUpdate(glm::vec2(loc.x, loc.z), glm::vec2(c->location.x, c->location.z), comp->entity);
				c->location = loc;

				ECS::Entity* ent = world->ecs->getEntity(comp->entity);
				LocationSystem* sys = reinterpret_cast<LocationSystem*>(system);
				if (ent->components.count(sys->rend_t->id))
				{
					for (auto&& kv : world->players)
					{
						RenderComponent* rend = reinterpret_cast<RenderComponent*>(world->ecs->getComponent(ent, sys->rend_t->id));
						auto change = world->makeChangeset<RenderChangeset>(0, rend->index);
						change->player = kv.first;
						change->dirty = true;
						world->queueChangeset(sys->rend_t, change);
					}
				}
			}
			auto pool = world->changesets[change->worker];
			pool->del<LocationChangeset>(change->id);
		});

		setWaitFunction([](ECS::System* system, World* world)
		{
			
		});
	};
private:
};