#pragma once
#include "../Macros.h"
#include "../Module.h"
#include "../events/Event.h"
#include "glm/glm.hpp"
#include <vector>
#include <unordered_map>

class Pipeline;
class World;

struct ObjectState
{
	double interpAlpha;

	glm::vec3 coordinatesCurrent;
	glm::vec3 coordinatesLast;

	glm::vec3 scaleCurrent;
	glm::vec3 scaleLast;

	glm::vec3 rotationCurrent;
	glm::vec3 rotationLast;
};

struct Object
{
	unsigned int id;

	std::string mesh;
	std::string model;
	std::string texture;
	unsigned int textureSlot;
	std::string shader;

	std::vector<ObjectState> instances;
};

class RenderRequestEvent : public EventData
{
	unsigned int entityId;
};

class ClientServerCommunication : public Module
{
public:
	V3API ClientServerCommunication();
	V3API ~ClientServerCommunication();

	void onStartup() override;

	V3API std::unordered_map<unsigned int, Object> getObjects(Object ob);

	void onTick() override;
	void onShutdown() override;
private:
	unsigned int objectIndex;
	std::vector<unsigned int> freeObjectIds;

	std::unordered_map<unsigned int, Object> objects;
};