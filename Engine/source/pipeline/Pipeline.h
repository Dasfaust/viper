#pragma once
#include "../view/ViewLayer.h"
#include "RenderCommand.h"
#include <boost/variant.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Pipeline : public Tickable
{
public:
	std::shared_ptr<tbb::concurrent_unordered_map<unsigned int, std::shared_ptr<RenderCommand>>> renderCommands;

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

		virtual void setUniform(std::string name, UniformValue value) = 0;
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

	class Camera
	{
	public:
		struct OnCameraMoveEventData
		{
			glm::vec3 cameraPos;
			glm::vec3 cameraFront;
			glm::vec3 cameraUp;
		};

		class OnCameraMoveEvent
		{
		public:
			tbb::concurrent_vector<std::shared_ptr<EventListener<OnCameraMoveEventData>>> listeners;
			
			OnCameraMoveEvent() { };

			void addListener(std::shared_ptr<EventListener<OnCameraMoveEventData>> listener)
			{
				listeners.push_back(listener);
			};

			void triggerEvent(std::weak_ptr<OnCameraMoveEventData> data)
			{
				for (auto& listener : listeners)
				{
					listener->callback(data.lock());
				}
			};

			void triggerEvent(std::shared_ptr<Camera> camera)
			{
				std::shared_ptr<OnCameraMoveEventData> data = std::make_shared<OnCameraMoveEventData>();

				data->cameraPos = camera->cameraPos;
				data->cameraFront = camera->cameraFront;
				data->cameraUp = camera->cameraUp;

				for (auto& listener : listeners)
				{
					listener->callback(data);
				}
			};
		};

		std::shared_ptr<OnCameraMoveEvent> moveEvent;

		glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
		glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

		glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), cameraUp);

		Camera()
		{
			moveEvent = std::make_shared<OnCameraMoveEvent>();
		};

		inline void update()
		{
			view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		};
	};

	inline unsigned int addRenderCommand(std::shared_ptr<RenderCommand> command)
	{
		unsigned int id = renderCommands->size();
		(*renderCommands)[id] = command;
		return id;
	};

	inline std::shared_ptr<RenderCommand> getRenderCommand(int id)
	{
		return (*renderCommands)[id];
	};

	std::shared_ptr<Camera> camera;
	std::atomic<double> alpha;

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