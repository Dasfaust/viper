#pragma once
#include "../world/ECS.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

class CameraComponent : public Component<CameraComponent>
{
public:
	float moveSpeed = 2.0f;
	float moveSensitivity = 0.08f;
	float zoomSpeed = 5.0f;
	float zoomSensitivity = 0.5f;
	float minZoom = 3.0f;
	float maxZoom = 50.0f;

	glm::vec3 floorPosition = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 floorPositionTarget = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 floorFront = glm::vec3(0.0f, -2.0f, -1.0f);
	glm::vec3 position = glm::vec3(0.0f, 3.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 front = glm::vec3(0.0f, -2.0f, -1.0f);
	float fov = 90.0f;
	float yaw = 45.0f;
	float pitch = -70.0f;
	float roll = 0.0f;
	float angle = 0.0f;
	float distance = 6.0f;

	glm::mat4 projection = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);

	glm::vec2 viewportSize;
	glm::vec2 mouseViewport;
	glm::vec2 _mouseViewport;
	glm::vec3 mouseWorld;
};

class CameraUpdater : public ComponentTicker<CameraComponent>
{
public:
};