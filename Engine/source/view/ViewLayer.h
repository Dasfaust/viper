#pragma once
#include "../Macros.h"
#include "../config/ConfigLayer.h"
#include "../events/EventLayer.h"
#include "../Tickable.h"

#ifndef V3_GLFW_INCLUDE
//#define GLFW_INCLUDE_VULKAN
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#endif

#include "glm/vec2.hpp"

namespace ViewEvents
{
	struct ButtonState
	{
		bool pressed = false;
		bool released = false;
	};

	/*struct OnMouseEventData
	{
		bool cancelled = false;
		std::unordered_map<int, ButtonState> buttons;
		glm::vec2 coordinates;
		glm::vec2 scroll;
	};

	class OnMouseEvent
	{
	public:
		tbb::concurrent_vector<std::shared_ptr<EventListener<OnMouseEventData>>> listeners;
		
		OnMouseEvent() { };

		void addListener(std::shared_ptr<EventListener<OnMouseEventData>> listener)
		{
			listeners.push_back(listener);
		};

		void triggerEvent(OnMouseEventData& data)
		{
			for (auto& listener : listeners)
			{
				listener->callback(data);
			}
		};
	};

	struct OnKeyEventData
	{
		bool cancelled = false;
		std::unordered_map<int, ButtonState> buttons;
	};

	class OnKeyEvent
	{
	public:
		tbb::concurrent_vector<std::shared_ptr<EventListener<OnKeyEventData>>> listeners;
		
		OnKeyEvent() { };

		void addListener(std::shared_ptr<EventListener<OnKeyEventData>> listener)
		{
			listeners.push_back(listener);
		};

		void triggerEvent(OnKeyEventData& data)
		{
			for (auto& listener : listeners)
			{
				listener->callback(data);
			}
		};
	};*/

	static void mouseCallback(GLFWwindow* window, double x, double y);

	static void mouseScrollCallback(GLFWwindow* window, double x, double y);

	static std::shared_ptr<std::unordered_map<int, ButtonState>> checkButtonStates();
	static std::shared_ptr<std::unordered_map<int, ButtonState>> checkKeyStates();
}

class ViewLayer : public Tickable
{
public:
	float viewWidth = 0.0f;
	float viewHeight = 0.0f;
	
	std::shared_ptr<glm::vec2> mouseCoords;
	std::shared_ptr<glm::vec2> scrollCoords;
	std::shared_ptr<std::unordered_map<int, ViewEvents::ButtonState>> buttonStates;
	std::shared_ptr<std::unordered_map<int, ViewEvents::ButtonState>> keyStates;
	//std::shared_ptr<ViewEvents::OnMouseEvent> mouseEvent;
	//std::shared_ptr<ViewEvents::OnKeyEvent> keyEvent;

	V3API ViewLayer(std::shared_ptr<EventLayer> events, std::shared_ptr<ConfigLayer> config);
	V3API ~ViewLayer();

	bool V3API closeRequested();
	void V3API setTitle(std::string title);
	V3API GLFWwindow* getWindow();
	void V3API tick() override;
private:
	std::shared_ptr<ConfigLayer> config;
	std::shared_ptr<EventLayer> events;
	GLFWwindow* window;
};

