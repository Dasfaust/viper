#include "ViewLayer.h"
#include "../Logger.h"

ViewLayer::ViewLayer(std::shared_ptr<EventLayer> events, std::shared_ptr<ConfigLayer> config)
{
	this->config = config;
	this->events = events;

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	
	auto size = config->getInts("engine", "clientSize");
	window = glfwCreateWindow(size[0], size[1], "Window", nullptr, nullptr);
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
