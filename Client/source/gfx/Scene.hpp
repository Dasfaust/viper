#pragma once
#include "ecs/ECS.hpp"

struct LocationComponent
{
	glm::vec3 location;
};

struct MeshComponent
{
	std::string meshName;
	std::string shaderName;
};

class Scene : public Module, public Modular
{
public:
	std::shared_ptr<ecs::Container> container;
	bool firstUpdate = true;

	void onStart() override
	{
		container = initModule<ecs::Container>("container");
		container->registerComponent<MeshComponent>();
		container->registerComponent<LocationComponent>();

		for (auto&& kv : modules)
		{
			kv.second->onStart();
		}
	};

	void onTick() override
	{
		if (firstUpdate)
		{
			uint64 ent = container->makeEntity({ ecs::ComponentIDs<MeshComponent>::ID, ecs::ComponentIDs<LocationComponent>::ID });
			container->getComponent<MeshComponent>(ent)->meshName = "plane";
			container->getComponent<LocationComponent>(ent)->location = glm::vec3(0.0f);
		}

		tickModules();

		if (firstUpdate)
		{
			firstUpdate = false;
		}
	};

	void onShutdown() override
	{
		for (auto&& kv : modules)
		{
			kv.second->onShutdown();
		}
	};
};
