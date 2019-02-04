#include "ViewLayer.h"
#include "../Logger.h"

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
	viewWidth = size[0];
	viewHeight = size[1];
	window = glfwCreateWindow(size[0], size[1], "Window", nullptr, nullptr);
	glfwMakeContextCurrent(window);

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
}
