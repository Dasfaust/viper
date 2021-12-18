#pragma once
#include "ecs/Container.hpp"
#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include "../input/InputManager.hpp"

struct Transform3D
{
	vec3 position = vec3(0.0f);
	vec3 rotationAxis = vec3(1.0f, 0.0f, 0.0f);
	float rotation = 0.0f;
	vec3 scale = vec3(1.0f);
	bool isStatic = false;
	bool dirty = true;
};

struct Camera
{
	bool dirty = true;
};

struct OrthoCamera : Camera
{
	vec4 bounds;
};

struct PerspectiveCamera : OrthoCamera
{
	float fov = 65.0f;
	vec3 offset = vec3(0.0f, 0.0f, -3.0f);
	vec3 target = vec3(0.0f);
	vec3 upAxis = vec3(0.0f, 1.0f, 0.0f);
	vec3 frontAxis = vec3(0.0f, 0.0f, -1.0f);
	float pitch = 0.0f;
	float yaw = 0.0f;
	float roll = 0.0f;
	float zoom = 0.0f;
};

struct PlayerInput
{

};

struct RenderData
{
	uint32 materialId = 0;
	uint32 meshId = 0;
	bool dirty = true;
};

struct InstanceMap
{
	uint32 mesh;
	uint32 material;
	umap(uint64, uint32) entities;
	std::vector<mat4> instances;
	bool changed = false;
};

struct RenderInstance
{
	uint64 entity;
	uint64 instanceId;
	mat4 model;
};

class RenderSystem : public ecs::System
{
public:
	std::shared_ptr<ecs::Container> container;
	umap(uint64, InstanceMap) renderData;
	moodycamel::ConcurrentQueue<RenderInstance> instanceQueue;

	RenderSystem()
	{
		updateEntity = [](ecs::Entity* entity, std::shared_ptr<System> self, float dt)
		{
			auto rs = std::reinterpret_pointer_cast<RenderSystem>(self);
			auto rd = ecs::shift<RenderData>(entity);

			if (rd->dirty)
			{
				uint64 instanceId = (uint64) rd->materialId << 32 | rd->meshId;

				auto tc = ecs::shift<Transform3D>(entity);

				auto model = mat4(1.0f);
				model = glm::translate(model, tc->position);
				model = glm::rotate(model, glm::radians(tc->rotation), tc->rotationAxis);
				model = glm::scale(model, tc->scale);

				rs->instanceQueue.enqueue({ entity->id, instanceId, model });

				rd->dirty = false;
			}
		};
	};

	void onTickEnd() override
	{
		RenderInstance instance;
		while(instanceQueue.try_dequeue(instance))
		{
			InstanceMap* map;
			if (renderData.find(instance.instanceId) == renderData.end())
			{
				renderData[instance.instanceId] = InstanceMap();
				map = &renderData[instance.instanceId];
				map->mesh = (uint32)instance.instanceId;
				map->material = instance.instanceId >> 32;
			}
			else
			{
				map = &renderData[instance.instanceId];
			}

			uint32 location;
			if (map->entities.find(instance.entity) == map->entities.end())
			{
				location = (uint32)map->instances.size();
				map->entities[instance.entity] = location;
				map->instances.resize(location + 1);
			}
			else
			{
				location = map->entities[instance.entity];
			}

			map->instances[location] = instance.model;

			map->changed = true;
		}
	};
};

struct ViewProjectionMatrix
{
	mat4 view;
	mat4 proj;
};

class CameraSystem : public ecs::System
{
public:
	umap(uint64, ViewProjectionMatrix) matrices;
	std::shared_ptr<ecs::Container> container;
	std::shared_ptr<WindowManager> wm;
	umap(uint64, vec3) cursorToWorld;
	std::shared_ptr<InputManager> input;

	CameraSystem()
	{
		updateEntity = [](ecs::Entity* entity, std::shared_ptr<System> self, float dt)
		{
			auto sys = std::reinterpret_pointer_cast<CameraSystem>(self);

			auto cam = ecs::shift<PerspectiveCamera>(entity);
			if (cam->dirty)
			{
				auto tc = ecs::shift<Transform3D>(entity);

				sys->matrices[entity->id] =
				{
					glm::lookAt(tc->position + cam->offset, tc->position + cam->offset + cam->frontAxis, cam->upAxis),
					glm::perspective(glm::radians(cam->fov), cam->bounds.x / cam->bounds.y, cam->bounds.z, cam->bounds.w)
				};

				cam->dirty = false;
			}

			if (entity->components[ecs::ComponentIDs<PlayerInput>::ID])
			{
				//cursorToWorld[entity->id] = glm::unProject(vec3(sys->input->mousePos.x, sys->input->mousePos.y, 0.0f), sys->matrices[entity->id].model * sys->matrices[entity->id].view, vec4(0.0f, 0.0f, sys->wm->width, sys->wm->height));
			}
		};
	};
};

enum InputDirection2D
{
	UP = 0, DOWN = 1, LEFT = 2, RIGHT = 3
};

class PlayerInputSystem : public ecs::System
{
public:
	std::shared_ptr<ecs::Container> container;
	std::shared_ptr<WindowManager> wm;
	std::shared_ptr<InputManager> input;
	std::shared_ptr<Listener<ButtonPressedEvent>> buttonPressed;
	float speed = 1.5f;
	float sensitivity = 0.5f;
	float lastMouseX = 0.0f;
	float lastMouseY = 0.0f;
	bool captured = false;

	umap(InputDirection2D, uint32) inputBindings2d;

	PlayerInputSystem()
	{
		inputBindings2d[UP] = KEY_W;
		inputBindings2d[DOWN] = KEY_S;
		inputBindings2d[LEFT] = KEY_A;
		inputBindings2d[RIGHT] = KEY_D;

		updateEntity = [](ecs::Entity* entity, std::shared_ptr<System> self, float dt)
		{
			auto pi = std::reinterpret_pointer_cast<PlayerInputSystem>(self);

			bool changed = false;
			
			auto trans = ecs::shift<Transform3D>(entity);
			auto cam = ecs::shift<PerspectiveCamera>(entity);

			if (pi->captured && pi->input->isDown(KEY_ESCAPE))
			{
				pi->captured = false;
				pi->wm->setShowCursor(true);
			}

			if (pi->captured && (fabs(pi->lastMouseX - pi->input->mousePos.x) > 0.01f || fabs(pi->lastMouseY - pi->input->mousePos.y) > 0.01f))
			{
				float xOffset = (pi->input->mousePos.x - pi->lastMouseX) * pi->sensitivity;
				float yOffset = (pi->lastMouseY - pi->input->mousePos.y) * pi->sensitivity;
				cam->yaw += xOffset;
				cam->pitch += yOffset;

				if (cam->pitch > 89.0f) { cam->pitch = 89.0f; }
				if (cam->pitch < -89.0f) { cam->pitch = -89.0f; }

				vec3 direction;
				direction.x = cos(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
				direction.y = sin(glm::radians(cam->pitch));
				direction.z = sin(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
				direction = glm::normalize(direction);

				cam->frontAxis = direction;
				pi->lastMouseX = pi->input->mousePos.x;
				pi->lastMouseY = pi->input->mousePos.y;

				changed = true;
			}

			if (pi->input->isDown(pi->inputBindings2d[UP]))
			{
				trans->position += cam->frontAxis * pi->speed * dt;
				changed = true;
			}
			if (pi->input->isDown(pi->inputBindings2d[DOWN]))
			{
				trans->position -= cam->frontAxis * pi->speed * dt;
				changed = true;
			}
			if (pi->input->isDown(pi->inputBindings2d[LEFT]))
			{
				trans->position += glm::normalize(glm::cross(cam->upAxis, cam->frontAxis)) * pi->speed * dt;
				changed = true;
			}
			if (pi->input->isDown(pi->inputBindings2d[RIGHT]))
			{
				trans->position -= glm::normalize(glm::cross(cam->upAxis, cam->frontAxis)) * pi->speed * dt;
				changed = true;
			}

			cam->target = trans->position;

			if (changed)
			{
				ecs::shift<PerspectiveCamera>(entity)->dirty = true;
			}
		};
	};

	void onStart() override
	{
		buttonPressed = wm->buttonPressedEvent->listen(20, [](ButtonPressedEvent& ev, std::vector<std::shared_ptr<Module>> modules)
		{
			auto self = std::reinterpret_pointer_cast<PlayerInputSystem>(modules[0]);
			if (ev.button == 0 && !self->captured)
			{
				self->lastMouseX = self->input->mousePos.x;
				self->lastMouseY = self->input->mousePos.y;
				self->captured = true;
				self->wm->setShowCursor(false);
				self->lastMouseX = self->wm->showCursorCoords.x;
				self->lastMouseY = self->wm->showCursorCoords.y;
			}
		}, { shared_from_this() });
	};

	void onTickBegin() override
	{
		buttonPressed->poll();
	};
};

struct ChunkCellPair
{
	int64 chunk;
	std::vector<uint32> cells;
};

struct CellUpdate
{
	uint64 entity;
	std::vector<ChunkCellPair> chunks;
};

class TransformSystem : public ecs::System
{
public:
	uint32 chunkSize = 16;
	// chunk id, [cell id][ [entity] ]
	umap(int64, std::vector<std::set<uint64>>) chunks;
	moodycamel::ConcurrentQueue<CellUpdate> updates;

	TransformSystem()
	{
		updateEntity = [](ecs::Entity* entity, std::shared_ptr<System> self, float dt)
		{
			auto ts = std::static_pointer_cast<TransformSystem>(self);
			auto transform = ecs::shift<Transform3D>(entity);
			if (transform->dirty)
			{
				CellUpdate update;
				update.entity = entity->id;
				
				int xFloor = (int)floor(fabs(transform->position.x));
				int zFloor = (int)floor(fabs(transform->position.z));
				
				int cellX = xFloor & 15;
				int cellZ = zFloor & 15;
				uint32 cellId = cellZ * ts->chunkSize + cellX;

				int chunkX = (int)(xFloor / ts->chunkSize) * transform->position.x > 0 ? 1 : -1;
				int chunkZ = (int)(zFloor / ts->chunkSize) * transform->position.z > 0 ? 1 : -1;
				int64 chunkId = (int64)chunkZ << 32 | chunkX;

				debug("Entity %llu: XYZ: (%.2f, %.2f, %.2f) Chunk: %lld (%d, %d), Cell: %d (%d, %d)", entity->id, transform->position.x, transform->position.y, transform->position.z, chunkId, chunkX, chunkZ, cellId, cellX, cellZ);

				update.chunks.push_back({ chunkId, { cellId } });
				ts->updates.enqueue(update);
				
				transform->dirty = false;
			}
		};
	};

	void onTickEnd() override
	{
		CellUpdate update;
		while(updates.try_dequeue(update))
		{
			for (auto pair : update.chunks)
			{
				if (!chunks.count(pair.chunk))
				{
					chunks[pair.chunk] = {};
					chunks[pair.chunk].resize((int)pow(chunkSize, 2));
				}
				auto chunk = &chunks[pair.chunk];
				for (auto cell : pair.cells)
				{
					(*chunk)[cell].insert(update.entity);
				}
			}
		}

		/*for (auto&& chunk : chunks)
		{
			uint32 count = 0;
			for (auto set : chunk.second)
			{
				count += set.size();
			}
			debug("Chunk %lld has %d entities", chunk.first, count);
		}*/
	};
};

class Scene : public Module, public Modular
{
public:
	static std::vector<uint64> players;
	std::shared_ptr<ecs::Container> container;
	std::shared_ptr<WindowManager> wm;
	std::shared_ptr<InputManager> input;
	std::shared_ptr<RenderSystem> renderSystem;
	std::shared_ptr<CameraSystem> cameraSystem;
	std::shared_ptr<PlayerInputSystem> playerInputSystem;
	std::shared_ptr<TransformSystem> transforms;
	bool firstUpdate = true;

	void onStart() override
	{
		wm = getParent<Module>()->getParent<Modular>()->getModule<WindowManager>("wm");
		input = getParent<Module>()->getParent<Modular>()->getModule<InputManager>("input");

		container = initModule<ecs::Container>("container");
		container->async = true;
		
		container->registerComponent<RenderData>();
		container->registerComponent<OrthoCamera>();
		container->registerComponent<PerspectiveCamera>();
		container->registerComponent<PlayerInput>();
		container->registerComponent<Transform3D>();

		for (auto&& kv : modules)
		{
			kv.second->onStart();
		}

		renderSystem = container->initSystem<RenderSystem>({ { ecs::ComponentIDs<RenderData>::ID, ecs::ComponentFlags::E_REQUIRED } });
		renderSystem->container = container;

		cameraSystem = container->initSystem<CameraSystem>({ { ecs::ComponentIDs<OrthoCamera>::ID, ecs::ComponentFlags::E_OPTIONAL }, { ecs::ComponentIDs<PerspectiveCamera>::ID, ecs::ComponentFlags::E_OPTIONAL } });
		cameraSystem->container = container;
		cameraSystem->wm = wm;

		playerInputSystem = container->initSystem<PlayerInputSystem>({ { ecs::ComponentIDs<PlayerInput>::ID, ecs::ComponentFlags::E_REQUIRED } });
		playerInputSystem->container = container;
		playerInputSystem->wm = wm;
		playerInputSystem->input = input;

		 container->initSystem({ { ecs::ComponentIDs<Transform3D>::ID, ecs::ComponentFlags::E_REQUIRED }, { ecs::ComponentIDs<PlayerInput>::ID, ecs::ComponentFlags::E_SKIP } }, [](ecs::Entity* entity, std::shared_ptr<ecs::System> self, float dt)
		{
			auto transform = ecs::shift<Transform3D>(entity);
			float rot = transform->rotation + (10.0f * dt);
			if (rot >= 180.0f) { rot = -180.0f; }
			transform->rotation = rot;
			auto rd = ecs::shift<RenderData>(entity);
			rd->dirty = true;
		});

		transforms = container->initSystem<TransformSystem>({ { ecs::ComponentIDs<Transform3D>::ID, ecs::ComponentFlags::E_REQUIRED } });
		
		initDefaultPlayer();

		auto genChunk = [&](int chunkX, int chunkY, uint32 size)
		{
			for (uint32 x = 0; x < size; x++)
			{
				for (uint32 y = 0; y < size; y++)
				{
					int worldX = chunkX * size + x;
					int worldY = chunkY * size + y;

					uint64 ent = container->makeEntity({ ecs::ComponentIDs<Transform3D>::ID, ecs::ComponentIDs<RenderData>::ID });
					auto rd = container->getComponent<RenderData>(ent);
					rd->materialId = 0;
					rd->meshId = 0;
					auto tc = container->getComponent<Transform3D>(ent);
					tc->position = vec3(worldX, 0, worldY);
					tc->rotationAxis = vec3(1.0f, 0.3f, 0.5f);
					tc->rotation = 0.0f;
					tc->scale = vec3(0.5f);
				}
			}
		};

		for (uint32 x = 0; x < 100; x++)
		{
			for (uint32 z = 0; z < 100; z++)
			{
				for (uint32 y = 0; y < 10; y++)
				{
					uint64 ent = container->makeEntity({ ecs::ComponentIDs<Transform3D>::ID, ecs::ComponentIDs<RenderData>::ID });
					auto rd = container->getComponent<RenderData>(ent);
					rd->materialId = 0;
					rd->meshId = 0;
					auto tc = container->getComponent<Transform3D>(ent);
					tc->position = vec3(x, y, z);
					tc->rotationAxis = vec3(1.0f, 0.3f, 0.5f);
					tc->rotation = 0.0f;
					tc->scale = vec3(0.5f);
				}
			}
		}

		//genChunk(1, 1, 16);
		//genChunk(1, 1, 16);
		//genChunk(-1, -1, 16);
	};

	void onTick() override
	{
		tickModules();
	};

	void initDefaultPlayer()
	{
		players.push_back(
		container->makeEntity(
			{
					ecs::ComponentIDs<Transform3D>::ID,
					ecs::ComponentIDs<PerspectiveCamera>::ID,
					ecs::ComponentIDs<PlayerInput>::ID
				}
			)
		);
		auto trans = container->getComponent<Transform3D>(players[0]);
		auto cam = container->getComponent<PerspectiveCamera>(players[0]);
		cam->bounds = vec4(wm->width, wm->height, 0.1f, 100.0f);
		cam->fov = 65.0f;
		trans->position = vec3(-1.5f, 1.0f, 1.5f);
		cam->target = trans->position;
		cam->frontAxis = vec3(0.6f, -0.3f, 0.6f);
	};

	ViewProjectionMatrix* getDefaultCamera()
	{
		if (players.empty()) { throw std::invalid_argument("The default camera has not been initialized"); }
		return &cameraSystem->matrices[players[0]];
	};

	void onShutdown() override
	{
		for (auto&& kv : modules)
		{
			kv.second->onShutdown();
		}
	};
};
