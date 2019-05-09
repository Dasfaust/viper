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
			if (/*!comp->stopped &&*/ entity->components.count(sys->loc_t->id))
			{
				/*loc = container->getComponent<LocationComponent>(entity);

				int x = floor(loc->location.x);
				int y = floor(loc->location.z);

				// TODO: get movement direction

				bool canMove = true;

				for (auto&& kv : world->getNearbyEntities2D(entity, 5))
				{
					for (auto e : kv.second)
					{
						if (kv.first == DIRECTION::WEST && e->components.count(sys->veh_t->id))
						{
							debugf("Collision at %d", kv.first);
							canMove = false;
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
					velocity.z -= 0.015;

					ECS::Changeset change = { loc->index, 0, velocity };
					world->changesets[loc->type_id].enqueue(change);
				}
				else
				{
					ECS::Changeset change = { true };
					world->changesets[comp->type_id].enqueue(change);
				}*/

				auto map = world->getNearbyEntities2D(entity, 1);
				for (auto&& kv : map)
				{
					for (auto c : kv.second)
					{
						auto e = container->getEntity(c.entity);

						auto _loc = reinterpret_cast<LocationComponent*>(container->getComponent(e, sys->loc_t->id));
						debugf("Nearby: (%.2f, %.2f): %d, dir: %s", floor(_loc->location.x), floor(_loc->location.z), e->index, kv.first.c_str());
					}
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
		if (entities.size() > 0)
		{
			entities.push_back(world->createEntity<LocationComponent, VehicleMotionComponent, MeshComponent, RenderComponent>([](uint32 index, ECS::Component *comp, std::vector<boost::any> vars)
			{
				if (index == 0)
				{
					auto c = reinterpret_cast<LocationComponent*>(comp);
					c->location.x = 3.0f;
					c->location.z = 3.0f;
				}
				else if (index == 1)
				{
					auto c = reinterpret_cast<VehicleMotionComponent*>(comp);
					c->stopped = true;
				}
				else if (index == 2)
				{
					auto c = reinterpret_cast<MeshComponent*>(comp);
					c->mesh = std::string("CubeMesh");
				}
			})->get());

			// west
			entities.push_back(world->createEntity<LocationComponent, MeshComponent, BoundingBox2D, RenderComponent>([](uint32 index, ECS::Component *comp, std::vector<boost::any> vars)
			{
				if (index == 0)
				{
					auto c = reinterpret_cast<LocationComponent*>(comp);
					c->location.x = 5.0f;
					c->location.z = 3.0f;
				}
				else if (index == 1)
				{
					auto c = reinterpret_cast<MeshComponent*>(comp);
					c->mesh = std::string("CubeMesh");
				}
				else if (index == 2)
				{
					auto c = reinterpret_cast<BoundingBox2D*>(comp);
					c->bounds = glm::vec2(0.0);
				}
			})->get());

			// east
			entities.push_back(world->createEntity<LocationComponent, MeshComponent, BoundingBox2D, RenderComponent>([](uint32 index, ECS::Component *comp, std::vector<boost::any> vars)
			{
				if (index == 0)
				{
					auto c = reinterpret_cast<LocationComponent*>(comp);
					c->location.x = 1.0f;
					c->location.z = 3.0f;
				}
				else if (index == 1)
				{
					auto c = reinterpret_cast<MeshComponent*>(comp);
					c->mesh = std::string("CubeMesh");
				}
				else if (index == 2)
				{
					auto c = reinterpret_cast<BoundingBox2D*>(comp);
					c->bounds = glm::vec2(0.0);
				}
			})->get());

			// south
			entities.push_back(world->createEntity<LocationComponent, MeshComponent, BoundingBox2D, RenderComponent>([](uint32 index, ECS::Component *comp, std::vector<boost::any> vars)
			{
				if (index == 0)
				{
					auto c = reinterpret_cast<LocationComponent*>(comp);
					c->location.x = 3.0f;
					c->location.z = 5.0f;
				}
				else if (index == 1)
				{
					auto c = reinterpret_cast<MeshComponent*>(comp);
					c->mesh = std::string("CubeMesh");
				}
				else if (index == 2)
				{
					auto c = reinterpret_cast<BoundingBox2D*>(comp);
					c->bounds = glm::vec2(0.5);
				}
			})->get());

			// north
			entities.push_back(world->createEntity<LocationComponent, MeshComponent, RenderComponent>([](uint32 index, ECS::Component *comp, std::vector<boost::any> vars)
			{
				if (index == 0)
				{
					auto c = reinterpret_cast<LocationComponent*>(comp);
					c->location.x = 3.0f;
					c->location.z = 1.0f;
				}
				else if (index == 1)
				{
					auto c = reinterpret_cast<MeshComponent*>(comp);
					c->mesh = std::string("CubeMesh");
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