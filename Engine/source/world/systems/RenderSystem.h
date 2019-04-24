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
	std::shared_ptr<moodycamel::ConcurrentQueue<RenderInfo>> dirtyObjects;

	std::shared_ptr<ECS::TypeInfo> loc_t;
	std::shared_ptr<ECS::TypeInfo> mesh_t;

	double updateMs = 0.0;
	double lastUpdate = 0.0;

	inline void init(ECS::Container* container, World* world) override
	{
		loc_t = container->resolveType<LocationComponent>();
		mesh_t = container->resolveType<MeshComponent>();
		dirtyObjects = std::make_shared<moodycamel::ConcurrentQueue<RenderInfo>>();

		setTickFunction([](double dt, ECS::Component* component, ECS::System* system, ECS::Container* container, World* world)
		{
			auto ts = tnow();
			auto sys = reinterpret_cast<RenderSystem*>(system);
			ECS::Entity* entity = container->getEntity(component->entity);

			//debugf("Ticking component: %d, entity %d", component->index, component->entity);

			LocationComponent* loc = nullptr;
			if (entity->components.count(sys->loc_t->id))
			{
				loc = container->getComponent<LocationComponent>(entity);
			}

			MeshComponent* mesh = nullptr;
			if (entity->components.count(sys->mesh_t->id))
			{
				mesh = container->getComponent<MeshComponent>(entity);
			}

			if (loc != 0 && mesh != 0)
			{
				while (!sys->dirtyObjects->try_enqueue({ entity->index, loc->location, mesh->mesh }));
			}
			//debugf("Render component tick took %.2f ms", tnow() - ts);
		});

		setWaitFunction([](ECS::System* system, World* world)
		{
			auto sys = reinterpret_cast<RenderSystem*>(system);
			// I don't even know how this would happen
			if (sys->dirtyObjects == 0)
			{
				return;
			}

			//rapidjson::Document js;
			//rapidjson::Value arr(rapidjson::kArrayType);
			//js.SetObject();
			auto ts = tnow();
			js::JsonObj js;
			std::vector<js::JsonObj> instances;
			RenderInfo info;
			while(sys->dirtyObjects->try_dequeue(info))
			{
				/*rapidjson::Value item;
				item.SetObject();
				item.AddMember("entity", info.index, js.GetAllocator());
				item.AddMember("mesh", rapidjson::StringRef(info.mesh.c_str()), js.GetAllocator());
				rapidjson::Value l;
				l.SetObject();
				l.AddMember("x", info.location.x, js.GetAllocator());
				l.AddMember("y", info.location.y, js.GetAllocator());
				l.AddMember("z", info.location.z, js.GetAllocator());
				item.AddMember("location", l, js.GetAllocator());
				arr.PushBack(item, js.GetAllocator());*/
				js::JsonObj i;
				js::set(i, "entity", js::i(info.index));
				js::set(i, "mesh", js::s(info.mesh));
				js::JsonObj l;
				js::set(l, "x", js::f(info.location.x));
				js::set(l, "y", js::f(info.location.y));
				js::set(l, "z", js::f(info.location.z));
				js::set(i, "location", l);
				instances.push_back(i);
			}
			//js.AddMember("instances", arr, js.GetAllocator());
			js::set(js, "instances", instances);

			if (instances.size() > 0)
			{
				sys->updateMs = tnow() - sys->lastUpdate;
				//js.AddMember("call", 3, js.GetAllocator());
				js::set(js, "call", js::i(3));
				sys->v3->getModule<Networking>()->send(js);
				sys->lastUpdate = tnow();
				//debugf("Ms since last state update: %.2f", sys->updateMs);
			}
		});
	};
private:
};
