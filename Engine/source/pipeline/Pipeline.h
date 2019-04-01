#pragma once
#include "../view/ViewLayer.h"
#include "../Module.h"
#include <boost/variant.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <boost/container/flat_map.hpp>
#include "../world/systems/Components.h"

class V3;

class Pipeline : public Module
{
public:
	struct Instance
	{
		LocationComponent location;
	};

	struct Player
	{
		MovementInputComponent movement;
		CameraComponent camera;
	};

	struct WorldState
	{
		bool initialized = false;
		boost::container::flat_map<MeshComponent, boost::container::flat_map<uint32, Instance>> state;
		Player player;
		double stepAlpha;
	};

	WorldState worldStateCurrent;
	WorldState worldStateLast;

	glm::mat4 projection = glm::mat4(1.0f);

	moodycamel::ConcurrentQueue<WorldState> stateUpdates;

	inline void updateState(WorldState state)
	{
		stateUpdates.enqueue(state);
	};

	class Shader
	{
	public:
		static const int UNIFORM_INT = 0;
		static const int UNIFORM_FLOAT = 1;
		static const int UNIFORM_VEC2 = 2;
		static const int UNIFORM_VEC3 = 3;
		static const int UNIFORM_VEC4 = 4;
		static const int UNIFORM_MAT4 = 5;

		typedef boost::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat4> UniformValue;

		std::string name;
		int id = -1;

		V3API virtual void setUniform(std::string name, UniformValue value) = 0;
	};

	class Mesh
	{
	public:
		std::string name;
		std::vector<float> vertices;
		std::vector<unsigned int> attribLayout;
		std::vector<unsigned int> indices;
	};

	double deltaTime = 0.0;

	class Model
	{
	public:
		std::string name;
		std::vector<std::string> meshes;
		std::vector<unsigned int> instances;
	};

	class Texture
	{
	public:
		std::string name;
		unsigned int id;
		std::string filePath;
		unsigned char* buffer;
		int width, height, bpp;
	};

	virtual void tick() override {  };

	virtual inline void createShader(std::string name)
	{
		throw std::runtime_error("createShader not implemented in current pipeline.");
	};

	virtual inline void meshToMemory(std::string name)
	{
		throw std::runtime_error("meshToMemory not implemented in current pipeline.");
	};
	
	virtual inline void modelToMemory(std::string name)
	{
		throw std::runtime_error("modelToMemory not implemented in current pipeline.");
	};

	virtual inline void textureToMemory(std::string name)
	{
		throw std::runtime_error("textureToMemory not implemented in current pipeline.");
	};

	virtual inline void meshToVRAM(std::string name)
	{
		throw std::runtime_error("meshToVRAM not implemented in current pipeline.");
	};

	virtual inline void modelToVRAM(std::string name)
	{
		throw std::runtime_error("modelToMemory not implemented in current pipeline.");
	};

	virtual inline void textureToVRAM(std::string name)
	{
		throw std::runtime_error("textureToVRAM not implemented in current pipeline.");
	};

	virtual inline unsigned int makeRenderCommand()
	{
		throw std::runtime_error("makeRenderCommand not implemented in current pipeline.");
	};
};