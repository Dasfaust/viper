#include "ViewLayer.h"
#include "../Logger.h"
#include "../V3.h"
#include "../config/ConfigLayer.h"
#include "../world/World.h"

static boost::container::flat_map<int, ViewEvents::ButtonState> ViewEvents::checkButtonStates()
{
	boost::container::flat_map<int, ViewEvents::ButtonState> states;

	for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++)
	{
		ButtonState state;
		if (instance->buttonStates->size() < i + 1 ||
			(glfwGetMouseButton(instance->getWindow(), i) == GLFW_PRESS &&
			(*instance->buttonStates)[i].pressed != true))
		{
			state.pressed = true;
			state.released = false;
			(*instance->buttonStates)[i] = state;
			states[i] = state;
		}
		if (instance->buttonStates->size() < i + 1 ||
			(glfwGetMouseButton(instance->getWindow(), i) == GLFW_RELEASE &&
			(*instance->buttonStates)[i].released != true))
		{
			state.pressed = false;
			state.released = true;
			(*instance->buttonStates)[i] = state;
			states[i] = state;
		}
	}

	return states;
}

static boost::container::flat_map<int, ViewEvents::ButtonState> ViewEvents::checkKeyStates()
{
	boost::container::flat_map<int, ViewEvents::ButtonState> states;

	for (int i = 0; i < GLFW_KEY_LAST; i++)
	{
		ButtonState state;
		if (instance->keyStates->find(i) == instance->keyStates->end() ||
			(glfwGetKey(instance->getWindow(), i) == GLFW_PRESS &&
			(*instance->keyStates)[i].pressed != true))
		{
			state.pressed = true;
			state.released = false;
			(*instance->keyStates)[i] = state;
			states[i] = state;
		}
		if (instance->keyStates->find(i) == instance->keyStates->end() ||
			(glfwGetKey(instance->getWindow(), i) == GLFW_RELEASE &&
			(*instance->keyStates)[i].released != true))
		{
			state.pressed = false;
			state.released = true;
			(*instance->keyStates)[i] = state;
			states[i] = state;
		}
	}

	return states;
}

// Only called when mouse moves
void ViewEvents::mouseCallback(GLFWwindow* window, double x, double y)
{
	auto event = instance->mouseEvent;
	auto data = event->makeEvent();
	data->cursor_coordinates = glm::vec2(x, y);
	data->scroll_wheel_coordinates = (*instance->scrollCoords);
	if ((*instance->mouseCoords) != data->cursor_coordinates)
	{
		instance->mouseCoords->x = data->cursor_coordinates.x;
		instance->mouseCoords->y = data->cursor_coordinates.y;
	}
	data->button_states = checkButtonStates();
	event->fire(data);
}

void ViewEvents::mouseScrollCallback(GLFWwindow* window, double x, double y)
{
	auto event = instance->mouseEvent;
	auto data = event->makeEvent();
	data->cursor_coordinates = (*instance->mouseCoords);
	data->scroll_wheel_coordinates = glm::vec2(x, y);
	if ((*instance->scrollCoords) != data->scroll_wheel_coordinates)
	{
		instance->scrollCoords->x = data->scroll_wheel_coordinates.x;
		instance->scrollCoords->y = data->scroll_wheel_coordinates.y;
	}
	data->button_states = checkButtonStates();
	event->fire(data);
}

ViewLayer::ViewLayer()
{

}

void ViewLayer::onStartup()
{
	glfwInit();
	//glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	ViewEvents::instance = v3->getModule<ViewLayer>();

	auto config = v3->getModule<ConfigLayer>();
	auto size = config->getInts("engine", "clientSize");
	viewWidth = (float)size[0];
	viewHeight = (float)size[1];
	window = glfwCreateWindow(size[0], size[1], "Window", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	mouseEvent = std::make_shared<Event<ViewEvents::MouseEvent>>();
	mouseCoords = std::make_shared<glm::vec2>();
	scrollCoords = std::make_shared<glm::vec2>();
	keyEvent = std::make_shared<Event<ViewEvents::KeyEvent>>();
	buttonStates = std::make_shared<boost::container::flat_map<int, ViewEvents::ButtonState>>();
	keyStates = std::make_shared<boost::container::flat_map<int, ViewEvents::ButtonState>>();
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
	glfwSetWindowTitle(window, (applicationName + title).c_str());
}

void ViewLayer::setApplicationName(std::string name)
{
	applicationName = name;
}

GLFWwindow* ViewLayer::getWindow()
{
	return window;
}

void ViewLayer::onTick()
{
	glfwPollEvents();
	
	auto mouse = ViewEvents::checkButtonStates();
	if (mouse.size() > 0)
	{
		auto data = mouseEvent->makeEvent();
		data->button_states = mouse;
		mouseEvent->fire(data);
	}

	auto keys = ViewEvents::checkKeyStates();
	if (keys.size() > 0)
	{
		auto data = keyEvent->makeEvent();
		data->buttons = keys;
		keyEvent->fire(data);
	}

#ifdef V3_DEBUG
	auto dt = v3->getModule<DeltaTime>();
	//auto wd = v3->getModule<World>();
	setTitle(" -> FPS: " + std::to_string(dt->framesPerSecond) /*+ " TPS: " + std::to_string(wd->stepsPerSecond)*/);
#else
	setTitle("");
#endif

	if (glfwWindowShouldClose(window))
	{
		v3->shutdown();
	}
}
