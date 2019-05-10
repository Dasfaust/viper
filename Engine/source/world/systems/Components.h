#pragma once
#include "../../ecs/ECS.h"
#include "glm/glm.hpp"
#include "boost/uuid/uuid.hpp"
#include <boost/functional/hash.hpp>

struct RenderState
{
	bool isDirty = true;
	bool hidden = false;
	int ticks = 0;
};

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
	boost::container::flat_map<boost::uuids::uuid, RenderState> states;
};

struct BoundingBox2D : public ECS::Component
{
	glm::vec2 bounds;
};

struct CollisionComponent : public ECS::Component
{ };

namespace Components
{
	static void registerTypes(ECS::Container* container)
	{
		container->resolveType<MovementInputComponent>();
		container->resolveType<LocationComponent>();
		container->resolveType<RenderComponent>();
		container->resolveType<MeshComponent>();
		container->resolveType<BoundingBox2D>();
		container->resolveType<CollisionComponent>();
	};
}