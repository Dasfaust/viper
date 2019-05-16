#include "World.h"
#include "../V3.h"
#include "../config/ConfigLayer.h"
#include "systems/MovementInputSystem.h"
#include "../ecs/ECSCommands.h"
#include "../networking/Networking.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/vector_angle.hpp"

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

	for (unsigned int i = 0; i < stepThreadCount; i++)
	{
		changesets[i] = std::make_shared<Pool>();
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

	loc_t = ecs->resolveType<LocationComponent>();
	bb_t = ecs->resolveType<BoundingBox2D>();

	pool = std::make_shared<Pool>();
	unsigned int ent = makeEntity();

	this->start();
	debug("World startup complete");
}

std::string World::getDirection2D(glm::vec2 pos1, glm::vec2 pos2)
{
	float angle = atan2(pos1.y - pos2.y, pos1.x - pos2.x);
	int octant = (int)round(8.0f * angle / (2.0f * glm::pi<float>()) + 8.0f) % 8;
	return headings[octant];
}

// TODO: this doesn't do negative values
std::unordered_map<std::string, std::vector<MapCell>> World::getNearbyEntities2D(ECS::Entity* ent, unsigned int radius)
{
	profiler_begin("nearby_2d");

	std::unordered_map<std::string, std::vector<MapCell>> _map;

	if (ent->components.count(loc_t->id))
	{
		LocationComponent* loc = reinterpret_cast<LocationComponent*>(ecs->getComponent(ent, loc_t->id));
		int x = (int)floor(loc->location.x);
		int y = (int)floor(loc->location.z);

		debugf("Searching around e%d with radius: %d", ent->index, radius);
		int swx = x - radius;
		int swy = y - radius;
		for (int i = (swx >= 0 ? swx : 0); i <= (x + radius >= 0 ? x + radius : 0); i++)
		{
			for (int j = (swy >= 0 ? swy : 0); j <= (y + radius >= 0 ? y + radius : 0); j++)
			{
				int id = (j < 0 ? 0 : j) * mapWidth + (i < 0 ? 0 : i);
				if (!map.count(id))
				{
					debugf("Out of bounds (%d, %d)", i, j);
					continue;
				}
				//debugf("Looking at (%d, %d): %d ents", i, j, map[id].size());
				for (auto cell : map[id])
				{
					if (cell.entity != ent->index)
					{
						std::string heading = getDirection2D(glm::vec2(loc->location.x, loc->location.z), glm::vec2(cell.position.x, cell.position.y));
						//debugf("e%d direction: %s", cell.entity, heading.c_str());

						bool found = false;
						for (auto c : _map[heading])
						{
							if (c.entity == cell.entity)
							{
								found = true;
								break;
							}
						}

						if (!found)
						{
							_map[heading].push_back(cell);
						}
					}
				}
			}
		}
	}

	profiler_end("nearby_2d");

	return _map;
}

void World::queueMapUpdate(glm::vec2 now, glm::vec2 last, unsigned int entity)
{
	mapGridUpdates.enqueue({ entity, now, last });
}

void World::tick()
{
	profiler_begin("world_tick");

	if (!loaded && !firstTick)
	{
		int mapSize = (mapWidth * mapHeight);
		float progress = 0.0f;
		int cellsLoaded = 1;
		for (int x = 0; x < mapWidth; x++)
		{
			for (int y = 0; y < mapHeight; y++)
			{
				int cell = y * mapWidth + x;
				map[cell] = { };

				glm::vec2 pos((float)x, (float)y);

				/*createEntity<LocationComponent, MeshComponent, RenderComponent>([](uint32 index, ECS::Component *comp, std::vector<boost::any> vars)
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
				}, { pos });*/

				cellsLoaded++;
				progress = (float)cellsLoaded / (float)mapSize;

				js::JsonObj js;
				js::set(js, "call", js::i(0));
				js::set(js, "progress", js::f(progress));
				v3->getModule<Networking>()->send(js);

				v3->getModule<Networking>()->_onTick();
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
				if (!changesetQueue.count(type->id))
				{
					changesetQueue[type->id] = moodycamel::ConcurrentQueue<ECS::Changeset*>();
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

	if (loaded)
	{
		profiler_begin("map_grid_update");
		MapCell cell;
		while (mapGridUpdates.try_dequeue(cell))
		{
			auto entity = ecs->getEntity(cell.entity);

			glm::vec2 bounds(0.0f);
			if (entity->components.count(bb_t->id))
			{
				auto bb = reinterpret_cast<BoundingBox2D*>(ecs->getComponent(entity, bb_t->id));
				bounds = bb->bounds;
			}

			// Clear old positions
			if (cell.position != cell.positionLast)
			{
				glm::vec2 sw(floor(cell.positionLast.x - bounds.x), floor(cell.positionLast.y - bounds.y));
				glm::vec2 ne(ceil(cell.positionLast.x + bounds.x), ceil(cell.positionLast.y + bounds.y));

				for (float i = (sw.x >= 0.0f ? sw.x : 0); i <= (ne.x >= 0.0f ? ne.x : 0.0f); i++)
				{
					for (float j = (sw.y >= 0.0f ? sw.y : 0.0f); j <= (ne.y >= 0 ? ne.y : 0.0f); j++)
					{
						int id = j * mapWidth + i;

						if (map.count(id))
						{
							for (int k = 0; k < map[id].size(); k++)
							{
								MapCell c = map[id][k];
								if (c.entity == entity->index)
								{
									map[id].erase(map[id].begin() + k);
								}
							}
						}
					}
				}
			}

			// Update new positions
			glm::vec2 sw(floor(cell.position.x - bounds.x), floor(cell.position.y - bounds.y));
			glm::vec2 ne(ceil(cell.position.x + bounds.x), ceil(cell.position.y + bounds.y));
			
			for (float i = (sw.x >= 0.0f ? sw.x : 0); i <= (ne.x >= 0.0f ? ne.x : 0.0f); i++)
			{
				for (float j = (sw.y >= 0.0f ? sw.y : 0.0f); j <= (ne.y >= 0 ? ne.y : 0.0f); j++)
				{
					int id = j * mapWidth + i;

					if (map.count(id))
					{
						// Insert into new id if it doesn't exist
						bool found = false;
						for (int k = 0; k < map[id].size(); k++)
						{
							MapCell c = map[id][k];
							if (c.entity == entity->index)
							{
								found = true;
							}
						}

						if (!found)
						{
							debugf("Map grid: updating e%d at (%.2f, %.2f) -> grid id: %d", cell.entity, i, j, id);
							map[id].push_back(cell);
						}
					}
				}
			}
		}
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

	for (auto&& kv : v3->getModule<Networking>()->players)
	{
		if (!players.count(kv.first))
		{
			players[kv.first] = { kv.second };
		}
	}

	profiler_begin("changesets");
	for (auto &&kv : changesetQueue)
	{
		ECS::Changeset* change;
		while ((&kv)->second.try_dequeue(change))
		{
			auto comp = reinterpret_cast<ECS::Component*>(&ecs->getHeap((&kv)->first)[change->index]);
			for (ECS::System* system : ecs->getSystems())
			{
				if (system->getTypes().count(comp->type_id))
				{
					system->applyChange("", comp, change, this, system);
					comp->lastChange = tnow();
				}
			}
		}
	}
	profiler_end("changesets");

	stepAccumulator += actualDeltaTime;
	auto stepBegin = tnow();
	if (!firstTick && nextStep <= stepBegin)
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
				if (system == nullptr || system == 0)
				{
					continue;
				}

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

					profiler_begin("job_wait");
					double jobWait = tnow();
					for (uint32 w : activeWorkers)
					{
						bool result;
						while (!workers[w]->finished.try_dequeue(result))
						{

						}
					}
					profiler_end("job_wait");
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

	profiler_begin("systems_tickwait");
	for (ECS::System* system : ecs->getSystems())
	{
		system->tickWait(system, this);
	}
	profiler_end("systems_tickwait");

	lastTickEnd = tnow();
	firstTick = false;

	v3->getModule<Networking>()->_onTick();

	profiler_end("world_tick");
}

void World::tickSystem(ECS::System* system, uint8 type, unsigned int worker, int start, int end)
{
	auto& heap = ecs->getHeap(type);
	size_t size = system->getTypes()[type];
	for (unsigned int i = start; i < (end > -1 ? end + size : (uint32)heap.size()); i += (unsigned int)size)
	{
		if (i == 0) continue;
		system->tickFunc(targetDeltaTime, reinterpret_cast<ECS::Component*>(&heap[i]), system, ecs, this, worker);
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