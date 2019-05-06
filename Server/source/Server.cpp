#include "V3.h"
#include "config/ConfigLayer.h"
#include "world/World.h"
#include "world/systems/RenderSystem.h"
#include "world/systems/LocationSystem.h"
#include "networking/Networking.h"
#include "console/ConsoleInput.h"
#include "util/Profiler.h"

struct VehicleMotionComponent : public ECS::Component
{
	bool stopped = false;
};

class VehicleMotionSystem : public ECS::System
{
	std::shared_ptr<ECS::TypeInfo> loc_t;
	std::shared_ptr<ECS::TypeInfo> veh_t;

	inline void init(ECS::Container* container, World* world) override
	{
		loc_t = container->resolveType<LocationComponent>();
		veh_t = container->resolveType<VehicleMotionComponent>();

		setTickFunction([](double dt, ECS::Component* component, ECS::System* system, ECS::Container* container, World* world)
		{
			auto sys = reinterpret_cast<VehicleMotionSystem*>(system);
			ECS::Entity* entity = container->getEntity(component->entity);
			auto comp = reinterpret_cast<VehicleMotionComponent*>(component);

			LocationComponent* loc = nullptr;
			if (!comp->stopped && entity->components.count(sys->loc_t->id))
			{
				loc = container->getComponent<LocationComponent>(entity);
				// x is z, y is x
				int x = floor(loc->location.x);
				int y = floor(loc->location.z);

				// TODO: get velocity movement direction

				// Check positive y
				bool canMove = true;
				for (int i = y; i < y + 3; i++)
				{
					int cell = i * world->mapWidth + x;
					for (auto c : world->map[cell])
					{
						if (c.entity != 0 && c.entity->index != entity->index && c.entity->components.count(sys->veh_t->id))
						{
							float dist = sqrt(pow(c.position.x - x, 2) + pow(c.position.y - y, 2) * 1.0f);
							debugf("Cell: %d e1 (%d, %d), Cell: %d e2 (%d, %d)", y * world->mapWidth + x, x, y, cell, c.position.x, c.position.y);
							debugf("Distance is %.2f", dist);
							if (dist <= 1.0f)
							{
								canMove = false;
								continue;
							}
						}
					}

					// Check negative y
					if (canMove)
					{
						for (int i = y; i > y - 3; i--)
						{
							int cell = i * world->mapWidth + x;
							for (auto c : world->map[cell])
							{
								if (c.entity != 0 && c.entity->index != entity->index && c.entity->components.count(sys->veh_t->id))
								{
									float dist = sqrt(pow(c.position.x - x, 2) + pow(c.position.y - y, 2) * 1.0f);
									debugf("Cell: %d e1 (%d, %d), Cell: %d e2 (%d, %d)", y * world->mapWidth + x, x, y, cell, c.position.x, c.position.y);
									debugf("Distance is %.2f", dist);
									if (dist <= 1.0f)
									{
										canMove = false;
										continue;
									}
								}
							}
						}
					}
				}

				if (canMove)
				{
					if (comp->stopped)
					{
						ECS::Changeset change = { false };
						world->changesets[comp->type_id].enqueue(change);
					}

					glm::vec3 velocity = loc->location;
					velocity.z += 0.015;

					ECS::Changeset change = { loc->index, 0, velocity };
					world->changesets[loc->type_id].enqueue(change);
				}
				else
				{
					ECS::Changeset change = { true };
					world->changesets[comp->type_id].enqueue(change);
				}
			}
		});

		setApplyChangeFunction([](std::string name, ECS::Component* comp, ECS::Changeset change)
		{
			auto c = reinterpret_cast<VehicleMotionComponent*>(comp);
			if (change.field == 0)
			{
				c->stopped = boost::any_cast<bool>(change.value);
			}
		});
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
			entities.push_back(world->createEntity<LocationComponent, VehicleMotionComponent, MeshComponent, RenderComponent>([](uint32 index, ECS::Component *comp, std::vector<boost::any> vars)
			{
				if (index == 0)
				{

				}
				else if (index == 2)
				{
					auto c = reinterpret_cast<MeshComponent*>(comp);
					c->mesh = std::string("CubeMesh");
				}
			})->get());

			entities.push_back(world->createEntity<LocationComponent, MeshComponent, RenderComponent, VehicleMotionComponent>([](uint32 index, ECS::Component *comp, std::vector<boost::any> vars)
			{
				if (index == 0)
				{
					auto c = reinterpret_cast<LocationComponent*>(comp);
					c->location.z = 7.0f;
				}
				else if (index == 1)
				{
					auto c = reinterpret_cast<MeshComponent*>(comp);
					c->mesh = std::string("CubeMesh");
				}
				else if (index == 3)
				{
					auto c = reinterpret_cast<VehicleMotionComponent*>(comp);
					c->stopped = true;
				}
			})->get());
		}
	};

	inline void onShutdown() override
	{
		
	};
};

int main()
{
	std::shared_ptr<V3> v3 = std::make_shared<V3>();
	//v3->initModule<Profiler>(1000.0);
	v3->initModule<ConsoleInput>();
	v3->initModule<ConfigLayer>();
	v3->initModule<Networking>();
	v3->initModule<World>();
	v3->initModule<Game>();
	v3->start();

    return EXIT_SUCCESS;
}