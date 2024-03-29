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

	void onStart() override
	{
		wm = getParent<Module>()->getParent<Modular>()->getModule<WindowManager>("wm");
		input = getParent<Module>()->getParent<Modular>()->getModule<InputManager>("input");

		container = initModule<ecs::Container>("container");
		container->async = false;
		container->registerComponent<RenderData>();
		container->registerComponent<Transform3D>();
		container->registerComponent<OrthoCamera>();
		container->registerComponent<PerspectiveCamera>();
		container->registerComponent<PlayerInput>();

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

		initDefaultPlayer();
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
