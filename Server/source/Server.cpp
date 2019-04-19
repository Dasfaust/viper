#include "V3.h"
#include "config/ConfigLayer.h"
#include "world/World.h"
#include "world/systems/RenderSystem.h"
#include "world/systems/LocationSystem.h"
#include "networking/Networking.h"
#include "console/ConsoleInput.h"

struct VehicleMotionComponent : public ECS::Component { };

class VehicleMotionSystem : public ECS::System
{
	std::shared_ptr<ECS::TypeInfo> loc_t;

	inline void init(ECS::Container* container, World* world) override
	{
		loc_t = container->resolveType<LocationComponent>();

		setTickFunction([](double dt, ECS::Component* component, ECS::System* system, ECS::Container* container, World* world)
		{
			auto sys = reinterpret_cast<VehicleMotionSystem*>(system);
			ECS::Entity* entity = container->getEntity(component->entity);
			auto comp = reinterpret_cast<VehicleMotionComponent*>(component);

			LocationComponent* loc = nullptr;
			if (entity->components.count(sys->loc_t->id))
			{
				loc = container->getComponent<LocationComponent>(entity);
				int x = floor(loc->location.x);
				int y = floor(loc->location.y);

				bool canMove = true;
				for (int i = x; i > x - 1; i--)
				{
					int cell = floor(loc->location.y) * world->mapWidth + x;
					for (auto c : world->map[cell])
					{
						if (c.entity != 0 && c.entity->index != entity->index)
						{
							canMove = false;
						}
					}
				}

				if (canMove)
				{
					glm::vec3 velocity = loc->location;
					velocity.z -= 0.002;

					ECS::Changeset change = { loc->index, 0, velocity };
					world->changesets[loc->type_id].enqueue(change);
				}
			}
		});

		setWaitFunction([](ECS::System* system, World* world) { });
	};
};

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
		world->createSystem<VehicleMotionSystem, VehicleMotionComponent>(1);
		world->createSystem<RenderSystem, RenderComponent>(2);
	};

	inline void onTick() override
	{
		if (entities.size() == 0)
		{
			/*for (int i = 0; i < 2; i++)
			{
				
			}*/

			entities.push_back(world->createEntity<LocationComponent, VehicleMotionComponent, RenderComponent>([](uint32 index, ECS::Component *comp)
			{
				if (index == 0)
				{
					/*auto c = reinterpret_cast<LocationComponent*>(comp);
					c->location.x = rand() % (30 - 1 + 1) + 1;*/
				}
			})->get());

			entities.push_back(world->createEntity<LocationComponent, RenderComponent>([](uint32 index, ECS::Component *comp)
			{
				if (index == 0)
				{
					auto c = reinterpret_cast<LocationComponent*>(comp);
					c->location.z = -200.0f;
				}
			})->get());

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