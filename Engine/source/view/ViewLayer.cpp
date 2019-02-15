#include "ViewLayer.h"
#include "../Logger.h"
#include "../V3.h"

static std::shared_ptr<std::unordered_map<int, ViewEvents::ButtonState>> ViewEvents::checkButtonStates()
{
	auto states = std::make_shared<std::unordered_map<int, ViewEvents::ButtonState>>();

	for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++)
	{
		ButtonState state;
		if (V3::getInstance()->getView()->buttonStates->size() < i + 1 ||
			(glfwGetMouseButton(V3::getInstance()->getView()->getWindow(), i) == GLFW_PRESS &&
			(*V3::getInstance()->getView()->buttonStates)[i].pressed != true))
		{
			state.pressed = true;
			state.released = false;
			(*V3::getInstance()->getView()->buttonStates)[i] = state;
			(*states)[i] = state;
		}
		if (V3::getInstance()->getView()->buttonStates->size() < i + 1 ||
			(glfwGetMouseButton(V3::getInstance()->getView()->getWindow(), i) == GLFW_RELEASE &&
			(*V3::getInstance()->getView()->buttonStates)[i].released != true))
		{
			state.pressed = false;
			state.released = true;
			(*V3::getInstance()->getView()->buttonStates)[i] = state;
			(*states)[i] = state;
		}
	}

	return states;
}

static std::shared_ptr<std::unordered_map<int, ViewEvents::ButtonState>> ViewEvents::checkKeyStates()
{
	auto states = std::make_shared<std::unordered_map<int, ViewEvents::ButtonState>>();

	for (int i = 0; i < GLFW_KEY_LAST; i++)
	{
		ButtonState state;
		if (V3::getInstance()->getView()->keyStates->find(i) == V3::getInstance()->getView()->keyStates->end() ||
			(glfwGetKey(V3::getInstance()->getView()->getWindow(), i) == GLFW_PRESS &&
			(*V3::getInstance()->getView()->keyStates)[i].pressed != true))
		{
			state.pressed = true;
			state.released = false;
			(*V3::getInstance()->getView()->keyStates)[i] = state;
			(*states)[i] = state;
		}
		if (V3::getInstance()->getView()->keyStates->find(i) == V3::getInstance()->getView()->keyStates->end() ||
			(glfwGetKey(V3::getInstance()->getView()->getWindow(), i) == GLFW_RELEASE &&
			(*V3::getInstance()->getView()->keyStates)[i].released != true))
		{
			state.pressed = false;
			state.released = true;
			(*V3::getInstance()->getView()->keyStates)[i] = state;
			(*states)[i] = state;
		}
	}

	return states;
}

// Only called when mouse moves
void ViewEvents::mouseCallback(GLFWwindow* window, double x, double y)
{
	auto data = std::make_shared<OnMouseEventData>();

	data->coordinates = glm::vec2(x, y);
	if ((*V3::getInstance()->getView()->mouseCoords) != data->coordinates)
	{
		V3::getInstance()->getView()->mouseCoords->x = data->coordinates.x;
		V3::getInstance()->getView()->mouseCoords->y = data->coordinates.y;
	}

	data->buttons = (*checkButtonStates());

	V3::getInstance()->getView()->mouseEvent->triggerEvent(data);
}

void ViewEvents::mouseScrollCallback(GLFWwindow* window, double x, double y)
{
	auto data = std::make_shared<OnMouseEventData>();
	data->scroll = glm::vec2(x, y);
	data->coordinates = (*V3::getInstance()->getView()->mouseCoords);
	data->buttons = (*checkButtonStates());
	V3::getInstance()->getView()->scrollCoords->x = (float)x;
	V3::getInstance()->getView()->scrollCoords->y = (float)y;
	V3::getInstance()->getView()->mouseEvent->triggerEvent(data);
}

ViewLayer::ViewLayer(std::shared_ptr<EventLayer> events, std::shared_ptr<ConfigLayer> config)
{
	this->config = config;
	this->events = events;

	glfwInit();
	//glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	auto size = config->getInts("engine", "clientSize");
	viewWidth = (float)size[0];
	viewHeight = (float)size[1];
	window = glfwCreateWindow(size[0], size[1], "Window", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	mouseEvent = std::make_shared<ViewEvents::OnMouseEvent>();
	mouseCoords = std::make_shared<glm::vec2>();
	scrollCoords = std::make_shared<glm::vec2>();
	keyEvent = std::make_shared<ViewEvents::OnKeyEvent>();
	buttonStates = std::make_shared<std::unordered_map<int, ViewEvents::ButtonState>>();
	keyStates = std::make_shared<std::unordered_map<int, ViewEvents::ButtonState>>();
	glfwSetCursorPosCallback(window, ViewEvents::mouseCallback);
	glfwSetScrollCallback(window, ViewEvents::mouseScrollCallback);

	glewInit();
}

ViewLayer::~ViewLayer()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

bool ViewLayer::closeRequested()
{
	return glfwWindowShouldClose(window);
}

void ViewLayer::setTitle(std::string title)
{
	glfwSetWindowTitle(window, title.c_str());
}

GLFWwindow* ViewLayer::getWindow()
{
	return window;
}

void ViewLayer::tick()
{
	glfwPollEvents();

	auto buttons = ViewEvents::checkButtonStates();
	if (buttons->size() > 0)
	{
		auto data = std::make_shared<ViewEvents::OnMouseEventData>();

		data->coordinates = (*mouseCoords);

		for (auto&& kv : (*buttons))
		{
			data->buttons[kv.first] = kv.second;
		}

		mouseEvent->triggerEvent(data);
	}
	
	auto keys = ViewEvents::checkKeyStates();
	if (keys->size() > 0)
	{
		auto data = std::make_shared<ViewEvents::OnKeyEventData>();

		for (auto&& kv : (*keys))
		{
			data->buttons[kv.first] = kv.second;
		}

		keyEvent->triggerEvent(data);
	}
}
