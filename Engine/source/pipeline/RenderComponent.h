#pragma once
#include "../world/ECS.h"
#include "glm/glm.hpp"

class LocationComponent : public Component<LocationComponent>
{
public:
	glm::vec3 locationLast;
	glm::vec3 locationCurrent;
	glm::vec3 scaleLast;
	glm::vec3 scaleCurrent;
	glm::vec3 locationLast;
	glm::vec3 rotationCurrent;
	std::pair<glm::vec3, glm::vec3> bounds;
};

class RenderComponent : public Component<RenderComponent>
{
public:
	std::string mesh;
	std::string model;
	std::string texture;
	unsigned int textureSlot;
	std::string shader;
	LocationComponent* location;
};