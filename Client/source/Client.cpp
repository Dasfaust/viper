#include "V3.h"
#include "pipeline/Pipeline.h"
#include "view/ViewLayer.h"
#include "config/ConfigLayer.h"
#include "world/World.h"
#include "pipeline/PipelineVk.h"
#include "world/systems/MovementInputSystem.h"
#include "world/systems/LocationSystem.h"
#include "world/systems/RenderSystem.h"

class Game : public V3Application
{
public:
	World* world;
	std::vector<ECS::Entity*> entities;

	inline void onStartup() override
	{
		//v3->getModule<ViewLayer>()->setApplicationName("A Game of Life");
		debugf("V3Application: onStartup");

		world = v3->getModule<World>();

		world->createSystem<MovementInputSystem, MovementInputComponent>(0);
		world->createSystem<LocationSystem, LocationComponent>(1);
		world->createSystem<RenderSystem, RenderComponent>(2);
	};

	inline void onTick() override
	{
		if (entities.size() == 0)
		{
			entities.push_back(world->createEntity<MovementInputComponent, LocationComponent, MeshComponent, CameraComponent>([](uint32 index, ECS::Component* comp)
			{
				if (index == 1)
				{
					auto loc = reinterpret_cast<LocationComponent*>(comp);
					//loc->location.y = 3.14f;
				}
				else if (index == 2)
				{
					auto mesh = reinterpret_cast<MeshComponent*>(comp);
					// Loses scope without constructor ¯\_(ツ)_/¯
					mesh->mesh = std::string("cube");
					mesh->texture = std::string("lewd");
					mesh->shader = std::string("basic");
					mesh->textureSlot = 0;
					mesh->makeHash();
					debugf("MeshComponent hash: %d", mesh->hash);
				}
				else if (index == 3)
				{
					auto cam = reinterpret_cast<CameraComponent*>(comp);
					cam->location = glm::vec3(0.0f, 3.0f, 0.0f);
				}
			})->get());
			debugf("Client: successful entity creation: %d", entities[0]->index);
		}
	};

	inline void onShutdown() override
	{
		
	};
};

int main()
{
	std::shared_ptr<V3> v3 = std::make_shared<V3>();
	v3->initModule<ConfigLayer>();
	//v3->initModule<ViewLayer>();
	//v3->initModule<PipelineVk>();
	v3->initModule<World>();
	v3->initModule<Game>();
	v3->start();

    return EXIT_SUCCESS;
}