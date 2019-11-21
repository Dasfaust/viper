#pragma once
#include "interface/Modular.hpp"
#include "VkGfx.hpp"
#include "Scene.hpp"

class Renderer : public Modular, public Module
{
public:
	std::shared_ptr<WindowManager> wm;
	std::shared_ptr<VkGfx> gfx;
	std::shared_ptr<Scene> scene;
	uint32 fps = 0;

	void onStart() override;

	void onTick() override;

	void onShutdown() override;
};

class FPSReporter : public Module
{
public:
	void onTick() override;
};