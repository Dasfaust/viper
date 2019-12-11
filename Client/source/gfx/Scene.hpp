#pragma once
#include "ecs/ECS.hpp"
#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include "../input/InputManager.hpp"

struct Transform2D
{
	vec2 position = vec2(0.0f);
	float rotation = 0.0f;
	float scale = 1.0f;
};

struct Transform3D
{
	vec3 position = vec3(0.0f);
	vec3 rotationAxis = vec3(1.0f, 0.0f, 0.0f);
	float rotation = 0.0f;
	vec3 scale = vec3(1.0f);
	bool isStatic = false;
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
			auto rd = reinterpret_cast<RenderData*>(entity->componentPointers[ecs::ComponentIDs<RenderData>::ID]);

			if (rd->dirty)
			{
				uint64 instanceId = (uint64) rd->materialId << 32 | rd->meshId;

				auto tc = reinterpret_cast<Transform3D*>(entity->componentPointers[ecs::ComponentIDs<Transform3D>::ID]);

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

	CameraSystem()
	{
		updateEntity = [](ecs::Entity* entity, std::shared_ptr<System> self, float dt)
		{
			auto sys = std::reinterpret_pointer_cast<CameraSystem>(self);

			if (entity->componentPointers[ecs::ComponentIDs<OrthoCamera>::ID])
			{
				auto cam = reinterpret_cast<OrthoCamera*>(entity->componentPointers[ecs::ComponentIDs<OrthoCamera>::ID]);
				if (cam->dirty)
				{
					auto tc = reinterpret_cast<Transform2D*>(entity->componentPointers[ecs::ComponentIDs<Transform2D>::ID]);

					float aspectRatio = (float)sys->wm->width / (float)sys->wm->height;

					mat4 transform = glm::translate(mat4(1.0f), vec3(tc->position, 0.0f)) * glm::rotate(mat4(1.0f), glm::radians(tc->rotation), vec3(0.0f, 0.0f, 1.0f));
					sys->matrices[entity->id] =
					{
						glm::inverse(transform),
						glm::ortho(cam->bounds[0] * aspectRatio, cam->bounds[1] * aspectRatio, cam->bounds[2], cam->bounds[3], -1.0f, 1.0f)
					};

					cam->dirty = false;
				}
			}
			else
			{
				auto cam = reinterpret_cast<PerspectiveCamera*>(entity->componentPointers[ecs::ComponentIDs<PerspectiveCamera>::ID]);
				if (cam->dirty)
				{
					auto tc = reinterpret_cast<Transform3D*>(entity->componentPointers[ecs::ComponentIDs<Transform3D>::ID]);

					sys->matrices[entity->id] =
					{
						glm::lookAt(tc->position + cam->offset, tc->position + cam->offset + cam->frontAxis, cam->upAxis),
						glm::perspective(glm::radians(cam->fov), cam->bounds.x / cam->bounds.y, cam->bounds.z, cam->bounds.w)
					};

					cam->dirty = false;
				}
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

			bool is2d = entity->componentPointers[ecs::ComponentIDs<Transform2D>::ID] != nullptr;

			bool changed = false;
			if (is2d)
			{
				auto trans = reinterpret_cast<Transform2D*>(entity->componentPointers[ecs::ComponentIDs<Transform2D>::ID]);

				if (pi->input->isDown(pi->inputBindings2d[UP]))
				{
					trans->position.y += pi->speed * dt;
					changed = true;
				}
				if (pi->input->isDown(pi->inputBindings2d[DOWN]))
				{
					trans->position.y -= pi->speed * dt;
					changed = true;
				}
				if (pi->input->isDown(pi->inputBindings2d[LEFT]))
				{
					trans->position.x -= pi->speed * dt;
					changed = true;
				}
				if (pi->input->isDown(pi->inputBindings2d[RIGHT]))
				{
					trans->position.x += pi->speed * dt;
					changed = true;
				}
			}
			else
			{
				auto trans = reinterpret_cast<Transform3D*>(entity->componentPointers[ecs::ComponentIDs<Transform3D>::ID]);
				auto cam = reinterpret_cast<PerspectiveCamera*>(entity->componentPointers[ecs::ComponentIDs<PerspectiveCamera>::ID]);

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
			}

			if (changed)
			{
				if (is2d)
				{
					reinterpret_cast<OrthoCamera*>(entity->componentPointers[ecs::ComponentIDs<OrthoCamera>::ID])->dirty = true;
				}
				else
				{
					reinterpret_cast<PerspectiveCamera*>(entity->componentPointers[ecs::ComponentIDs<PerspectiveCamera>::ID])->dirty = true;
				}
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
	bool firstUpdate = true;
	time_val updateTimeMs;

	void onStart() override
	{
		wm = getParent<Module>()->getParent<Modular>()->getModule<WindowManager>("wm");
		input = getParent<Module>()->getParent<Modular>()->getModule<InputManager>("input");

		container = initModule<ecs::Container>("container");
		container->registerComponent<RenderData>();
		container->registerComponent<Transform2D>();
		container->registerComponent<Transform3D>();
		container->registerComponent<OrthoCamera>();
		container->registerComponent<PerspectiveCamera>();
		container->registerComponent<PlayerInput>();

		for (auto&& kv : modules)
		{
			kv.second->onStart();
		}

		renderSystem = container->initSystem<RenderSystem>({ { ecs::ComponentIDs<RenderData>::ID, ecs::NONE_REQUIRED } });
		renderSystem->container = container;

		cameraSystem = container->initSystem<CameraSystem>({ { ecs::ComponentIDs<OrthoCamera>::ID, ecs::NOT_REQUIRED }, { ecs::ComponentIDs<PerspectiveCamera>::ID, ecs::NOT_REQUIRED } });
		cameraSystem->container = container;
		cameraSystem->wm = wm;

		playerInputSystem = container->initSystem<PlayerInputSystem>({ { ecs::ComponentIDs<PlayerInput>::ID, ecs::NONE_REQUIRED } });
		playerInputSystem->container = container;
		playerInputSystem->wm = wm;
		playerInputSystem->input = input;

		container->initSystem({ { ecs::ComponentIDs<Transform3D>::ID, ecs::NONE_REQUIRED }, { ecs::ComponentIDs<PlayerInput>::ID, ecs::SKIP } }, [](ecs::Entity* entity, std::shared_ptr<ecs::System> self, float dt)
		{
			auto transform = reinterpret_cast<Transform3D*>(entity->componentPointers[ecs::ComponentIDs<Transform3D>::ID]);
			float rot = transform->rotation + (10.0f * dt);
			if (rot >= 180.0f) { rot = -180.0f; }
			transform->rotation = rot;
			auto rd = reinterpret_cast<RenderData*>(entity->componentPointers[ecs::ComponentIDs<RenderData>::ID]);
			rd->dirty = true;
		});

		initDefaultPlayer();

		for (uint32 x = 0; x < 100; x++)
		{
			for (uint32 z = 0; z < 100; z++)
			{
				for (uint32 y = 0; y < 2; y++)
				{
					uint64 ent = makeEntity({ ecs::ComponentIDs<Transform3D>::ID, ecs::ComponentIDs<RenderData>::ID });
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

		uint64 ent = makeEntity({ ecs::ComponentIDs<Transform3D>::ID, ecs::ComponentIDs<RenderData>::ID });
		auto rd = container->getComponent<RenderData>(ent);
		rd->materialId = 1;
		rd->meshId = 1;
		auto tc = container->getComponent<Transform3D>(ent);
		tc->position = vec3(-1.0f, 0.0f, -1.0f);
		tc->rotationAxis = vec3(1.0f, 0.3f, 0.5f);
		tc->rotation = 0.0f;
		tc->scale = vec3(0.5f);
	};

	void onTick() override
	{
		auto start = tnowns();
		tickModules();
		updateTimeMs = timesince(start);
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

		/*auto cam = container->getComponent<OrthoCamera>(players[0]);
		cam->bounds = vec4(-1.0f, 1.0f, -1.0f, 1.0f);*/

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

	uint64 makeEntity(std::set<uint32> comps)
	{
		auto ent = container->makeEntity(comps);
		return ent;
	};

	void onShutdown() override
	{
		for (auto&& kv : modules)
		{
			kv.second->onShutdown();
		}
	};
};
