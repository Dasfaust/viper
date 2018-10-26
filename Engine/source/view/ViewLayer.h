#pragma once
#include "../config/ConfigLayer.h"
#include "../events/EventLayer.h"
#include "../Tickable.h"

#ifndef V3_GLFW_INCLUDE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

class ViewLayer : public Tickable
{
public:
	ViewLayer(std::shared_ptr<EventLayer> events, std::shared_ptr<ConfigLayer> config);
	~ViewLayer();

	bool closeRequested();
	void setTitle(std::string title);
	GLFWwindow* getWindow();
	void tick() override;
private:
	std::shared_ptr<ConfigLayer> config;
	std::shared_ptr<EventLayer> events;
	GLFWwindow* window;
};

