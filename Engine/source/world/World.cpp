#include "World.h"
#include "../V3.h"
#include "../config/ConfigLayer.h"
#include "systems/MovementInputSystem.h"
#include "../ecs/ECSCommands.h"
#include "../networking/Networking.h"
#include "../util/Profiler.h"

World::World()
{

}

World::~World()
{
	delete ecs;
}

void World::onStartup()
{
	ecs = new ECS::Container();
	Components::registerTypes(ecs);
	if (v3->isModuleLoaded<ConsoleInput>())
	{
		v3->getModule<ConsoleInput>()->registerCommand("ecs", std::make_shared<ECSCommand>());
	}

	auto config = v3->getModule<ConfigLayer>();
	stepThreadCount = config->getInts("engine", "worldStepThreads")[0];

	targetDeltaTime = 1000.0 / 30.0;
	debugf("World delta target: %.2f", targetDeltaTime);

	for (unsigned int i = 0; i < (unsigned int)stepThreadCount; i++)
	{
		queues.push_back(std::make_shared<moodycamel::ConcurrentQueue<bool>>());
		workers.push_back(std::make_shared<Worker>(this, i));
		workers[i]->start();
		debugf("Worker %d started", i + 1);
	}

	if (v3->isModuleLoaded<Networking>())
	{
		v3->getModule<Networking>()->callbacks[4] = [](Networking* net, NetworkPlayer& player, rapidjson::Document &data)
		{
			bool expected = data["speed"].GetBool();
			debugf("World paused: %d", expected);
			while (!net->v3->getModule<World>()->paused.compare_exchange_weak(expected, data["speed"].GetBool()));

			js::JsonObj rep;
			js::set(rep, "call", js::i(4));
			js::set(rep, "success", js::b(true));
			net->send(player, rep);
		};
	}

	this->start();
	debug("World startup complete");
}

void World::tick()
{
	profiler_begin("world_tick");

	if (!loaded && !firstTick)
	{
		int mapSize = mapWidth * mapHeight;
		float progress = 0.0f;
		int cellsLoaded = 1;
		for (int x = 0; x < mapWidth; x++)
		{
			for (int y = 0; y < mapHeight; y++)
			{
				int cell = y * mapWidth + x;
				map[cell] = { };

				glm::vec2 pos((float)x, (float)y);

				createEntity<LocationComponent, MeshComponent, RenderComponent>([](uint32 index, ECS::Component *comp, std::vector<boost::any> vars)
				{
					if (index == 0)
					{
						auto c = reinterpret_cast<LocationComponent*>(comp);
						c->location.x = boost::any_cast<glm::vec2>(vars[0]).x;
						c->location.z = boost::any_cast<glm::vec2>(vars[0]).y;
					}
					else if (index == 1)
					{
						auto c = reinterpret_cast<MeshComponent*>(comp);
						c->mesh = std::string("FlatMesh");
					}
				}, { pos });

				cellsLoaded++;
				progress = (float)cellsLoaded / (float)mapSize;

				js::JsonObj js;
				js::set(js, "call", js::i(0));
				js::set(js, "progress", js::f(progress));
				v3->getModule<Networking>()->send(js);
			}
		}

		bool expected = true;
		while (!loaded.compare_exchange_strong(expected, true));
	}

	double start = tnow();
	actualDeltaTime = start - lastTickEnd;

	std::pair<uint32, std::vector<std::shared_ptr<ECS::TypeInfo>>> sys;
	while (systemsToCreate.try_dequeue(sys))
	{
		auto s = ecs->createSystem(sys.second[0], sys.first, v3, this);
		for (auto type : sys.second)
		{
			if (type->id != sys.second[0]->id)
			{
				s->addType(type->id, type->size);
				if (!changesets.count(type->id))
				{
					changesets[type->id] = moodycamel::ConcurrentQueue<ECS::Changeset>();
				}
			}
		}
	}

	std::shared_ptr<Future<ECS::Entity*>> entFuture;
	while (entsToCreate.try_dequeue(entFuture))
	{
		auto entity = ecs->getEntity(ecs->createEntity());
		debugf("Created entity: %d", entity->index);
		for (uint32 i = 0; i < entFuture->components.size(); i++)
		{
			auto info = entFuture->components[i];
			uint32 id = ecs->createComponent(entity, info->id, info->size);
			debugf("Created component: id %d, type %d, size %d", id, info->id, info->size);
			if (entFuture->callback != nullptr)
			{
				entFuture->callback(i, ecs->getComponent(entity, info->id), entFuture->callbackVars);
			}
		}
		entFuture->fulfill(entity);
	}

	profiler_begin("map_grid_update");
	MapCell cell;
	while(mapGridUpdates.try_dequeue(cell))
	{
		int curId = floor(cell.position.y) * mapWidth + floor(cell.position.x);
		int lastId = floor(cell.positionLast.y) * mapWidth + floor(cell.positionLast.x);
		if (curId != lastId)
		{
			bool exists = false;
			for (int i = 0; i < map[lastId].size(); i++)
			{
				MapCell c = map[lastId][i];
				if (c.entity != 0 && c.entity->index == cell.entity->index)
				{
					map[lastId].erase(map[lastId].begin() + i);
				}
				
			}
			/*for (int i = 0; i < map[curId].size(); i++)
			{
				MapCell c = map[curId][i];
				if (c.entity != 0 && c.entity->index == cell.entity->index)
				{
					exists = true;
				}

			}
			if (!exists)
			{
				map[curId].push_back(cell);
			}*/
			map[curId].push_back(cell);
		}

		bool found = false;
		for (int i = 0; i < map[curId].size(); i++)
		{
			MapCell c = map[curId][i];
			if (c.entity != 0 && c.entity->index == cell.entity->index)
			{
				found = true;
			}
		}

		if (!found)
		{
			map[curId].push_back(cell);
		}
	}
	if (cell.entity != 0)
	{
		profiler_end("map_grid_update");
	}

	perfAccumulator += actualDeltaTime;
	if (perfAccumulator >= 500.0)
	{
		profiler_begin("server_telemetry");
		perfAccumulator = 0.0;

		if (v3->isModuleLoaded<Networking>())
		{
			auto net = v3->getModule<Networking>();
			if (net->clients > 0)
			{
				js::JsonObj js;
				js::set(js, "call", js::i(2));
				js::set(js, "sps", js::d(1000.0 / (actualStepDelta <= targetDeltaTime ? targetDeltaTime : actualStepDelta)));
				js::set(js, "sms", js::d(actualStepDelta));
				js::set(js, "swm", js::d(net->worldUpdateMs.load()));
				net->send(js);
			}
		}
		profiler_end("server_telemetry");
	}

	stepAccumulator += actualDeltaTime;
	auto stepBegin = tnow();
	if (nextStep <= stepBegin)
	{
		profiler_begin("world_step");

		// TODO: a better way of doing this
		bool canTick = true;
		if (v3->isModuleLoaded<Networking>())
		{
			canTick = !(v3->getModule<Networking>()->clients.load() == 0);
		}

		auto t = std::chrono::high_resolution_clock::now();

		if (canTick && !paused.load())
		{
			double jobsStart = tnow();
			for (ECS::System* system : ecs->getSystems())
			{
				std::vector<uint32> activeWorkers;
				for (auto kv : system->getTypes())
				{
					unsigned int elements = (unsigned int)((ecs->getHeap((&kv)->first).size() / (&kv)->second) - 1);

					if (elements >= (unsigned int)stepThreadCount)
					{
						unsigned int itemsPerWorker = elements / stepThreadCount;
						unsigned int lastIndex = 0;
						for (unsigned int i = 0; i < (unsigned int)stepThreadCount; i++)
						{
							int start = (int)(lastIndex + (&kv)->second);
							int end = (i + 1 == stepThreadCount ? -1 : (itemsPerWorker == 1 ? start : (lastIndex + (itemsPerWorker * (unsigned int)(&kv)->second))));
							lastIndex = end;

							workers[i]->queueJob({ system, (&kv)->first, start, end });
							activeWorkers.push_back(i);
						}
					}
					else
					{
						workers[0]->queueJob({ system, (&kv)->first, 0, -1 });
						activeWorkers.push_back(0);
					}

					for (auto queue : queues)
					{
						queue->enqueue(true);
					}

					double jobWait = tnow();
					for (uint32 w : activeWorkers)
					{
						bool result;
						while (!workers[w]->finished.try_dequeue(result))
						{

						}
					}
				}
			}
		}

		for (ECS::System* system : ecs->getSystems())
		{
			system->tickEnd(system, this);
		}

		double end = tnow();
		lastStepEnd = end;
		actualStepDelta = end - start;
		stepAccumulator = 0.0;

		nextStep = stepBegin + (1000.0 / 30.0);

		profiler_end("world_step");
	}

	double modificationStart = tnow();

	ECS::Entity* entity;
	while (entsToDelete.try_dequeue(entity))
	{
		ecs->deleteEntity(entity);
	}

	ECS::Component* component;
	while (compsToDelete.try_dequeue(component))
	{
		ecs->deleteComponent(component);
	}

	for (auto &&kv : changesets)
	{
		ECS::Changeset change;
		while((&kv)->second.try_dequeue(change))
		{
			// TODO: actually set the proper component, ya dingus
			auto comp = reinterpret_cast<ECS::Component*>(&ecs->getHeap((&kv)->first)[change.index]);
			for (ECS::System* system : ecs->getSystems())
			{
				if (system->getTypes().count(comp->type_id))
				{
					system->applyChange(ecs->resolveType(comp->type_id)->name, comp, change);
				}
			}
		}
	}

	for (ECS::System* system : ecs->getSystems())
	{
		system->tickWait(system, this);
	}

	lastTickEnd = tnow();
	firstTick = false;

	v3->getModule<Networking>()->_onTick();

	profiler_end("world_tick");
}

void World::tickSystem(ECS::System* system, uint8 type, int start, int end)
{
	auto& heap = ecs->getHeap(type);
	size_t size = system->getTypes()[type];
	for (unsigned int i = start; i < (end > -1 ? end + size : (uint32)heap.size()); i += (unsigned int)size)
	{
		if (i == 0) continue;
		system->tickFunc(targetDeltaTime, reinterpret_cast<ECS::Component*>(&heap[i]), system, ecs, this);
	}
}

void World::onShutdown()
{
	this->stop();
	for (auto worker : workers)
	{
		worker->stop();
	}
}