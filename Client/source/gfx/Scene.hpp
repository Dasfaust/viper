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
};

struct Camera
{
	bool dirty = true;
};

struct OrthoCamera : Camera
{
	vec4 bounds;
};

struct PlayerInput
{

};

struct RenderData
{
	std::string meshName = "";
	std::string materialName = "";
	bool dirty = true;
};

struct RenderInstance
{
	bool is3D = true;
	Transform2D transform2d;
	Transform3D transform3d;
};

typedef umap(uint64, RenderInstance) InstanceMap;

class RenderSystem : public ecs::System
{
public:
	std::shared_ptr<ecs::Container> container;
	umap(std::string, umap(std::string, InstanceMap)) renderData;

	RenderSystem()
	{
		updateEntity = [](ecs::Entity* entity, std::shared_ptr<System> self, float dt)
		{
			auto rs = std::reinterpret_pointer_cast<RenderSystem>(self);
			auto rd = reinterpret_cast<RenderData*>(entity->componentPointers[ecs::ComponentIDs<RenderData>::ID]);

			if (rd->dirty)
			{
				umap(std::string, InstanceMap)* mInstances;
				if (rs->renderData.find(rd->materialName) == rs->renderData.end())
				{
					rs->renderData[rd->materialName] = umap(std::string, umap(uint64, RenderInstance))();
				}
				mInstances = &rs->renderData[rd->materialName];

				InstanceMap* instances;
				if (mInstances->find(rd->meshName) == mInstances->end())
				{
					(*mInstances)[rd->meshName] = umap(uint64, RenderInstance)();
				}
				instances = &(*mInstances)[rd->meshName];

				RenderInstance* instance;
				if ((*instances).find(entity->id) == instances->end())
				{
					(*instances)[entity->id] = { };
				}
				instance = &(*instances)[entity->id];

				bool is2d = entity->componentPointers[ecs::ComponentIDs<Transform2D>::ID] != nullptr;

				if (is2d)
				{
					instance->is3D = false;
					instance->transform2d = Transform2D(*reinterpret_cast<Transform2D*>(entity->componentPointers[ecs::ComponentIDs<Transform2D>::ID]));
				}
				else
				{
					instance->transform3d = Transform3D(*reinterpret_cast<Transform3D*>(entity->componentPointers[ecs::ComponentIDs<Transform3D>::ID]));
				}

				rd->dirty = false;
			}
		};
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
				auto trans = pi->container->getComponent<Transform2D>(entity->id);

				if (pi->input->isDown(pi->inputBindings2d[UP]))
				{
					trans->position.y += 0.5f * dt;
					changed = true;
				}
				if (pi->input->isDown(pi->inputBindings2d[DOWN]))
				{
					trans->position.y -= 0.5f * dt;
					changed = true;
				}
				if (pi->input->isDown(pi->inputBindings2d[LEFT]))
				{
					trans->position.x -= 0.5f * dt;
					changed = true;
				}
				if (pi->input->isDown(pi->inputBindings2d[RIGHT]))
				{
					trans->position.x += 0.5f * dt;
					changed = true;
				}
			}
			else
			{
				
			}

			if (changed)
			{
				if (is2d)
				{
					reinterpret_cast<OrthoCamera*>(entity->componentPointers[ecs::ComponentIDs<OrthoCamera>::ID])->dirty = true;
				}
			}
		};
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
		container->registerComponent<PlayerInput>();

		for (auto&& kv : modules)
		{
			kv.second->onStart();
		}

		renderSystem = container->initSystem<RenderSystem>({ ecs::ComponentIDs<RenderData>::ID });
		renderSystem->container = container;

		cameraSystem = container->initSystem<CameraSystem>({ ecs::ComponentIDs<OrthoCamera>::ID });
		cameraSystem->container = container;
		cameraSystem->wm = wm;

		playerInputSystem = container->initSystem<PlayerInputSystem>({ ecs::ComponentIDs<PlayerInput>::ID });
		playerInputSystem->container = container;
		playerInputSystem->wm = wm;
		playerInputSystem->input = input;

		initDefaultPlayer();

		for (uint32 x = 0; x < 10; x++)
		{
			for (uint32 y = 0; y < 10; y++)
			{
				uint64 ent = makeEntity({ ecs::ComponentIDs<Transform2D>::ID, ecs::ComponentIDs<RenderData>::ID });
				auto rd = container->getComponent<RenderData>(ent);
				auto tf = container->getComponent<Transform2D>(ent);
				rd->meshName = "plane";
				rd->materialName = "flat";
				tf->position = glm::vec2((float)x * 0.5f, (float)y * 0.5f);
				tf->scale = 0.5f;
			}
		}

		uint64 ent = makeEntity({ ecs::ComponentIDs<Transform2D>::ID, ecs::ComponentIDs<RenderData>::ID });
		auto rd = container->getComponent<RenderData>(ent);
		rd->meshName = "plane_texture";
		rd->materialName = "basic";
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
					ecs::ComponentIDs<Transform2D>::ID,
					ecs::ComponentIDs<OrthoCamera>::ID,
					ecs::ComponentIDs<PlayerInput>::ID
				}
			)
		);
		auto trans = container->getComponent<Transform2D>(players[0]);
		trans->position = vec2(0.0f);
		trans->rotation = 0.0f;
		trans->scale = 1.0f;

		auto cam = container->getComponent<OrthoCamera>(players[0]);
		cam->bounds = vec4(-1.0f, 1.0f, -1.0f, 1.0f);
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
