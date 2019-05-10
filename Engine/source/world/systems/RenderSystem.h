#pragma once
#include "../World.h"
#include "Components.h"
#include "../../networking/Networking.h"
#include "../../V3.h"
#include <glm/vec3.hpp>

struct RenderInfo
{
	uint32 index;
	glm::vec3 location;
	std::string mesh;
};

class RenderSystem : public ECS::System
{
public:
	//std::shared_ptr<moodycamel::ConcurrentQueue<RenderInfo>> dirtyObjects;

	std::shared_ptr<ECS::TypeInfo> loc_t;
	std::shared_ptr<ECS::TypeInfo> mesh_t;
	std::shared_ptr<ECS::TypeInfo> rend_t;

	double updateMs = 0.0;
	double lastUpdate = 0.0;

	boost::container::flat_map<boost::uuids::uuid, moodycamel::ConcurrentQueue<RenderInfo>> playerQueues;

	inline void init(ECS::Container* container, World* world) override
	{
		loc_t = container->resolveType<LocationComponent>();
		mesh_t = container->resolveType<MeshComponent>();
		rend_t = container->resolveType<RenderComponent>();
		//dirtyObjects = std::make_shared<moodycamel::ConcurrentQueue<RenderInfo>>();

		setTickFunction([](double dt, ECS::Component* component, ECS::System* system, ECS::Container* container, World* world)
		{
			// It happens. Idk
			if (system == nullptr)
			{
				return;
			}
			auto ts = tnow();
			auto sys = reinterpret_cast<RenderSystem*>(system);
			auto comp = reinterpret_cast<RenderComponent*>(component);
			ECS::Entity* entity = container->getEntity(component->entity);

			//debugf("Ticking component: %d, entity %d", component->index, component->entity);

			LocationComponent* loc = nullptr;
			if (entity->components.count(sys->loc_t->id))
			{
				loc = reinterpret_cast<LocationComponent*>(container->getComponent(entity, sys->loc_t->id));
			}

			MeshComponent* mesh = nullptr;
			if (entity->components.count(sys->mesh_t->id))
			{
				mesh = reinterpret_cast<MeshComponent*>(container->getComponent(entity, sys->mesh_t->id));
			}

			RenderComponent* rend = nullptr;
			if (entity->components.count(sys->rend_t->id))
			{
				rend = reinterpret_cast<RenderComponent*>(container->getComponent(entity, sys->rend_t->id));
			}

			if (loc != 0 && mesh != 0 && rend != 0)
			{

				//while (!sys->dirtyObjects->try_enqueue({ entity->index, loc->location, mesh->mesh }));
				//world->queueChangeset(sys->rend_t, {comp->index, 0, any::make(false)});

				for (auto&& kv : world->players)
				{
					// TODO: check if is in rendering bounds
					RenderState state = rend->states[kv.first];
					/*if (rend->states.count(kv.first))
					{
						state = rend->states[kv.first];
					}
					else
					{
						debugf("Count failed for %d", comp->entity);
						while (!sys->playerQueues[kv.first].try_enqueue({ entity->index, loc->location, mesh->mesh }));
						Any a = any::make(false);
						a.uuid_val = kv.first;
						world->queueChangeset(sys->rend_t, { comp->index, 0, a });
						continue;
					}*/

					if (!state.hidden && state.isDirty)
					{
						if (!sys->playerQueues.count(kv.first))
						{
							sys->playerQueues[kv.first] = moodycamel::ConcurrentQueue<RenderInfo>();
						}

						debugf("Rendering %d", entity->index);
						while (!sys->playerQueues[kv.first].try_enqueue({ entity->index, loc->location, mesh->mesh }));
						if (loc->lastChange == 0.0 || tnow() - loc->lastChange >= (1000.0 / 30.0) * 4.0)
						{
							Any a = any::make(false);
							a.uuid_val = kv.first;
							world->queueChangeset(sys->rend_t, { comp->index, 0, a });
						}
					}
				}
			}
			//debugf("Render component tick took %.2f ms", tnow() - ts);
		});

		setTickEndFunction([](ECS::System* system, World* world)
		{
			auto sys = reinterpret_cast<RenderSystem*>(system);

			for (auto&& kv : sys->playerQueues)
			{
				auto ts = tnow();
				js::JsonObj js;
				std::vector<js::JsonObj> instances;
				RenderInfo info;
				while (kv.second.try_dequeue(info))
				{
					js::JsonObj i;
					js::set(i, "entity", js::i(info.index));
					js::set(i, "mesh", js::s(info.mesh));
					js::JsonObj l;
					js::set(l, "x", js::f(info.location.x));
					js::set(l, "y", js::f(info.location.y));
					js::set(l, "z", js::f(info.location.z));
					js::set(i, "location", l);
					js::set(i, "time", js::d(tnow()));
					instances.push_back(i);
				}
				js::set(js, "instances", instances);

				if (instances.size() > 0)
				{
					double end = tnow();
					sys->updateMs = end - sys->lastUpdate;
					js::set(js, "call", js::i(3));
					Networking* net = sys->v3->getModule<Networking>();
					net->send(net->players[kv.first], js);
					sys->lastUpdate = end;
					debugf("Ms since last state update: %.2f", sys->updateMs);
				}
			}
		});

		setApplyChangeFunction([](std::string name, ECS::Component* comp, ECS::Changeset change, World* world, System* system)
		{
			RenderComponent* c = reinterpret_cast<RenderComponent*>(comp);
			if (!c->states.count(any::uid(change.value)))
			{
				c->states[any::uid(change.value)] = { };
			}
			if (change.field == 0)
			{
				c->states[any::uid(change.value)].isDirty = any::b(change.value);
				c->states[any::uid(change.value)].ticks = 0;
				debugf("Dirty %d: %d", c->entity, any::b(change.value));
			}
		});
	};
private:
};
