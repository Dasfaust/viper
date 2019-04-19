#include "V3.h"
#include "config/ConfigLayer.h"
#include "world/World.h"
#include "world/systems/RenderSystem.h"
#include "world/systems/LocationSystem.h"
#include "networking/Networking.h"
#include "console/ConsoleInput.h"



class Game : public V3Application
{
public:
	World* world;
	std::vector<ECS::Entity*> entities;

	inline void onStartup() override
	{
		debugf("V3Application: onStartup");

		world = v3->getModule<World>();

		world->createSystem<LocationSystem, LocationComponent>(0);
		world->createSystem<RenderSystem, RenderComponent>(1);
	};

	inline void onTick() override
	{
		if (entities.size() == 0)
		{
			for (int i = 0; i < 2; i++)
			{
				entities.push_back(world->createEntity<LocationComponent, RenderComponent>([](uint32 index, ECS::Component *comp)
				{
					if (index == 0)
					{
						auto c = reinterpret_cast<LocationComponent*>(comp);
						c->location.x = rand() % (30 - 1 + 1) + 1;
					}
				})->get());
			}

			debug("Entities created");
		}
	};

	inline void onShutdown() override
	{
		
	};
};

int main()
{
	std::shared_ptr<V3> v3 = std::make_shared<V3>();
	v3->initModule<ConsoleInput>();
	v3->initModule<ConfigLayer>();
	v3->initModule<Networking>();
	v3->initModule<World>();
	v3->initModule<Game>();
	v3->start();

    return EXIT_SUCCESS;
}