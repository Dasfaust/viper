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
	std::string direction;
};

struct VehicleMotionChangeset : public ECS::Changeset
{
	bool stopped;
};

class VehicleMotionSystem : public ECS::System
{
	std::shared_ptr<ECS::TypeInfo> loc_t;
	std::shared_ptr<ECS::TypeInfo> veh_t;

	inline void init(ECS::Container* container, World* world) override
	{
		loc_t = container->resolveType<LocationComponent>();
		veh_t = container->resolveType<VehicleMotionComponent>();
		world->registerChangeset<VehicleMotionChangeset>();

		setTickFunction([](double dt, ECS::Component* component, ECS::System* system, ECS::Container* container, World* world, unsigned int worker)
		{
			auto sys = reinterpret_cast<VehicleMotionSystem*>(system);
			ECS::Entity* entity = container->getEntity(component->entity);
			auto comp = reinterpret_cast<VehicleMotionComponent*>(component);

			LocationComponent* loc = nullptr;
			if (!comp->stopped && entity->components.count(sys->loc_t->id))
			{
				loc = reinterpret_cast<LocationComponent*>(container->getComponent(entity, sys->loc_t->id));

				bool canMove = true;

				for (auto&& kv : world->getNearbyEntities2D(entity, 1))
				{
					for (auto c : kv.second)
					{
						auto e = container->getEntity(c.entity);

						auto _loc = reinterpret_cast<LocationComponent*>(container->getComponent(e, sys->loc_t->id));
						debugf("Nearby: (%.2f, %.2f): %d, dir: %s", _loc->location.x, _loc->location.z, e->index, kv.first.c_str());
						canMove = false;
					}
				}

				if (canMove)
				{
					if (comp->stopped)
					{
						/*ECS::Changeset change = { comp->index, 0, any::make(false) };
						world->changesets[comp->type_id].enqueue(change);*/
						auto change = world->makeChangeset<VehicleMotionChangeset>(worker, comp->index);
						change->stopped = false;
						world->queueChangeset(sys->veh_t, change);
					}

					glm::vec3 velocity = loc->location;
					debugf("Moving %s", comp->direction);
					velocity.z += comp->direction == "S" ? 0.015 : -0.015;

					/*ECS::Changeset change = { loc->index, 0, any::make(velocity) };
					world->changesets[loc->type_id].enqueue(change);*/
					auto change = world->makeChangeset<LocationChangeset>(worker, loc->index);
					change->location = velocity;
					world->queueChangeset(sys->loc_t, change);
				}
				else
				{
					/*ECS::Changeset change = { comp->index, 0, any::make(true) };
					world->changesets[comp->type_id].enqueue(change);*/
					auto change = world->makeChangeset<VehicleMotionChangeset>(worker, comp->index);
					change->stopped = true;
					world->queueChangeset(sys->veh_t, change);
				}
			}

			/*for (auto&& kv : world->getNearbyEntities2D(entity, 2))
			{
				for (auto c : kv.second)
				{
					auto e = container->getEntity(c.entity);

					auto _loc = reinterpret_cast<LocationComponent*>(container->getComponent(e, sys->loc_t->id));
					debugf("Nearby: (%.2f, %.2f): %d, dir: %s", _loc->location.x, _loc->location.z, e->index, kv.first.c_str());
				}
			}*/
		});

		setApplyChangeFunction([](std::string name, ECS::Component* comp, ECS::Changeset* change, World* world, System* system)
		{
			auto c = reinterpret_cast<VehicleMotionComponent*>(comp);
			if (change->field == 0)
			{
				c->stopped = reinterpret_cast<VehicleMotionChangeset*>(change)->stopped;
			}
			auto pool = world->changesets[change->worker];
			pool->del<VehicleMotionChangeset>(change->id);
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
			entities.push_back(world->createEntity<LocationComponent, VehicleMotionComponent, MeshComponent, RenderComponent>([](uint32 index, ECS::Component *comp, std::vector<Any> vars)
			{
				if (index == 0)
				{
					auto c = reinterpret_cast<LocationComponent*>(comp);
					c->location.x = 0.0f;
					c->location.z = 1.0f;
				}
				else if (index == 1)
				{
					auto c = reinterpret_cast<VehicleMotionComponent*>(comp);
					c->stopped = false;
					c->direction = std::string("S");
				}
				else if (index == 2)
				{
					auto c = reinterpret_cast<MeshComponent*>(comp);
					c->mesh = std::string("CubeMesh");
				}
			})->get());

			world->queueMapUpdate(glm::vec2(0.0f, 1.0f), glm::vec2(), entities[0]->index);

			entities.push_back(world->createEntity<LocationComponent, VehicleMotionComponent, MeshComponent, RenderComponent>([](uint32 index, ECS::Component *comp, std::vector<Any> vars)
			{
				if (index == 0)
				{
					auto c = reinterpret_cast<LocationComponent*>(comp);
					c->location.x = 0.0f;
					c->location.z = 6.0f;
				}
				else if (index == 1)
				{
					auto c = reinterpret_cast<VehicleMotionComponent*>(comp);
					c->stopped = false;
					c->direction = std::string("N");
				}
				else if (index == 2)
				{
					auto c = reinterpret_cast<MeshComponent*>(comp);
					c->mesh = std::string("CubeMesh");
				}
			})->get());

			world->queueMapUpdate(glm::vec2(0.0f, 6.0f), glm::vec2(), entities[1]->index);
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