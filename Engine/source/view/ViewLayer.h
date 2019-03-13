#pragma once
#include "../Macros.h"
#include "../Module.h"
#include "../events/Event.h"
#ifndef V3_GLFW_INCLUDE
//#define GLFW_INCLUDE_VULKAN
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#endif
#include "glm/vec2.hpp"
#include <unordered_map>

namespace ViewEvents
{
	struct ButtonState
	{
		bool pressed = false;
		bool released = false;
	};

	class MouseEvent : public EventData
	{
	public:
		std::unordered_map<int, ButtonState> button_states;
		glm::vec2 cursor_coordinates;
		glm::vec2 scroll_wheel_coordinates;
	};

	class KeyEvent : public EventData
	{
	public:
		std::unordered_map<int, ButtonState> buttons;
	};

	static void mouseCallback(GLFWwindow* window, double x, double y);

	static void mouseScrollCallback(GLFWwindow* window, double x, double y);

	static std::unordered_map<int, ButtonState> checkButtonStates();
	static std::unordered_map<int, ButtonState> checkKeyStates();
}

class ViewLayer : public Module
{
public:
	float viewWidth = 0.0f;
	float viewHeight = 0.0f;
	
	std::shared_ptr<glm::vec2> mouseCoords;
	std::shared_ptr<glm::vec2> scrollCoords;
	std::shared_ptr<std::unordered_map<int, ViewEvents::ButtonState>> buttonStates;
	std::shared_ptr<std::unordered_map<int, ViewEvents::ButtonState>> keyStates;

	std::shared_ptr<Event<ViewEvents::MouseEvent>> mouseEvent;
	std::shared_ptr<Event<ViewEvents::KeyEvent>> keyEvent;

	V3API ViewLayer();
	V3API ~ViewLayer();

	bool V3API closeRequested();
	void V3API setTitle(std::string title);
	void V3API setApplicationName(std::string name);
	V3API GLFWwindow* getWindow();
	void V3API onTick() override;

	void V3API onStartup() override;
private:
	GLFWwindow* window;
	std::string applicationName = "V3::";
};

namespace ViewEvents
{
	static ViewLayer* instance;
}

