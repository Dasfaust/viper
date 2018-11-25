#pragma once
#include "../Macros.h"
#include "../config/ConfigLayer.h"
#include "../events/EventLayer.h"
#include "../Tickable.h"

#ifndef V3_GLFW_INCLUDE
//#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

class ViewLayer : public Tickable
{
public:
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

