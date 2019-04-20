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

	inline void init(ECS::Container* container, World* world) override
	{
		loc_t = container->resolveType<LocationComponent>();
		mesh_t = container->resolveType<MeshComponent>();
		dirtyObjects = std::make_shared<moodycamel::ConcurrentQueue<RenderInfo>>();

		setTickFunction([](double dt, ECS::Component* component, ECS::System* system, ECS::Container* container, World* world)
		{
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
		});

		setWaitFunction([](ECS::System* system, World* world)
		{
			auto sys = reinterpret_cast<RenderSystem*>(system);
			// I don't even know how this would happen
			if (sys->dirtyObjects == 0)
			{
				return;
			}

			rapidjson::Document js;
			RenderInfo info;
			while(sys->dirtyObjects->try_dequeue(info))
			{
				rapidjson::Document item;
				item["entity"].SetInt(info.index);
				item["mesh"].SetString(rapidjson::StringRef(info.mesh.c_str()));
				item["location"]["x"] = info.location.x;
				item["location"]["y"] = info.location.y;
				item["location"]["z"] = info.location.z;
				js["instances"].push_back(item);
			}

			if (js.size() > 0)
			{
				js["call"] = 3;
				sys->v3->getModule<Networking>()->send(js);
			}
		});
	};
private:
};
