#pragma once
#include "../view/ViewLayer.h"
#include "RenderCommand.h"
#include <boost/variant.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../math/Math.h"

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

				/*data->cameraPos = camera->position;
				data->cameraFront = camera->direction;
				data->cameraUp = camera->up;*/

				for (auto& listener : listeners)
				{
					listener->callback(data);
				}
			};
		};

		std::shared_ptr<EventListener<ViewEvents::OnMouseEventData>> mouseListener;
		std::shared_ptr<EventListener<ViewEvents::OnKeyEventData>> keyListener;

		std::shared_ptr<OnCameraMoveEvent> moveEvent;

		float moveSpeed = 2.0f;
		float moveSensitivity = 0.08f;
		float zoomSpeed = 5.0f;
		float zoomSensitivity = 0.5f;
		float minZoom = 3.0f;
		float maxZoom = 50.0f;

		glm::vec3 floorPosition = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 floorPositionTarget = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 floorFront = glm::vec3(0.0f, -2.0f,  -1.0f);
		glm::vec3 position      = glm::vec3(0.0f, 3.0f,  0.0f);
		glm::vec3 up            = glm::vec3(0.0f, 1.0f,  0.0f);
		glm::vec3 front         = glm::vec3(0.0f, -2.0f,  -1.0f);
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

		Camera()
		{
			moveEvent = std::make_shared<OnCameraMoveEvent>();
		};

		void moveForward(double deltaTime)
		{
			/*floorPosition += ((float)(moveSpeed * (deltaTime / 1000.0f))) * glm::vec3(front.x, 0.0f, front.z);*/
			floorPositionTarget += moveSensitivity * glm::vec3(floorFront.x, 0.0f, floorFront.z);
			/*position.x = floorPosition.x;
			position.z = floorPosition.z;*/
		}

		void moveBack(double deltaTime)
		{
			/*floorPosition -= ((float)(moveSpeed * (deltaTime / 1000.0f))) * glm::vec3(front.x, 0.0f, front.z);
			position.x = floorPosition.x;
			position.z = floorPosition.z;*/
			floorPositionTarget -= moveSensitivity * glm::vec3(floorFront.x, 0.0f, floorFront.z);
		}

		void moveLeft(double deltaTime)
		{
			/*floorPosition -= glm::normalize(glm::cross(front, up)) * ((float)(moveSpeed * 1.5f * (deltaTime / 1000.0f)));
			position.x = floorPosition.x;
			position.z = floorPosition.z;*/
			floorPositionTarget -= glm::normalize(glm::cross(floorFront, up)) * moveSensitivity;
		}

		void moveRight(double deltaTime)
		{
			/*floorPosition += glm::normalize(glm::cross(front, up)) * ((float)(moveSpeed * 1.5f * (deltaTime / 1000.0f)));
			position.x = floorPosition.x;
			position.z = floorPosition.z;*/
			floorPositionTarget += glm::normalize(glm::cross(floorFront, up)) * moveSensitivity;
		}

		inline void update()
		{
			/*floorFront.x = cos(glm::radians(yaw));
			front.x = cos(glm::radians(pitch)) * floorFront.x + position.x;
			//front.y = sin(glm::radians(pitch));
			floorFront.z = sin(glm::radians(yaw));
			front.z = cos(glm::radians(pitch)) * floorFront.z + position.z;
			floorFront = glm::normalize(floorFront);*/

			view = glm::lookAt(position, position + glm::normalize(front), up);
			mouseWorld = cursorToWorld();
		};

		inline void pollEvents()
		{
			mouseListener->poll();
			keyListener->poll();
		}

		inline glm::vec3 cursorToWorld()
		{
			
			float xNormal = (2.0f * mouseViewport.x) / viewportSize.x - 1.0f;
			float yNormal = (2.0f * mouseViewport.y) / viewportSize.y - 1.0f;

			glm::vec4 clip = glm::vec4(xNormal, yNormal, -1.0f, 1.0f);

			glm::mat4 inverse = glm::inverse(projection);
			glm::vec4 eyes = inverse * clip;
			eyes.z = -1.0f; eyes.w = 0.0f;

			inverse = glm::inverse(view);
			glm::vec4 ray = inverse * eyes;

			

			return glm::normalize(glm::vec3(ray.x, ray.y, ray.z));
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
	std::atomic<double> timestepProgress;

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