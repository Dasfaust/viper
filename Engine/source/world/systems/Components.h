#pragma once
#include "../../ecs/ECS.h"
#include "glm/glm.hpp"

/*
	MovementInputSystem
	Update MovementInputComponent via listening to key events
*/
struct MovementInputComponent : public ECS::Component
{
	float right;
	float forward;
	float pitch;
	float yaw;
};

/*
	LocationSystem
	Get MovementInputComponent from entity, update location
*/
struct LocationComponent : public ECS::Component
{
	glm::vec3 location;
};

// Collision?

struct MeshComponent : public ECS::Component
{
	std::string mesh;
};

/*
	RenderSystem
	Entity marked for rendering
*/
struct RenderComponent : public ECS::Component
{
	bool dirty = true;
};

/*
	RenderSystem
	Get LocationComponent from entity, update camera accordingly
*/
struct CameraComponent : public ECS::Component
{
	glm::vec3 location = glm::vec3(0.0f, 3.0f, 0.0f);
};

namespace Components
{
	static void registerTypes(ECS::Container* container)
	{
		container->resolveType<MovementInputComponent>();
		container->resolveType<LocationComponent>();
		container->resolveType<RenderComponent>();
		container->resolveType<CameraComponent>();
		container->resolveType<MeshComponent>();
	};
}