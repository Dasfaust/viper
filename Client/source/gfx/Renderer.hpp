#pragma once
#include "interface/Modular.hpp"
#include "Scene.hpp"
#include "../surface/WindowManager.hpp"
#include "../ui/UIManager.hpp"
#include "interface/Pipeline.hpp"

enum GFX_API
{
	OPEN_GL,
	VULKAN
};
static const char* GFX_API_NAMES[] = { "OpenGL", "Vulkan" };

class Renderer : public Modular, public Module
{
public:
	static GFX_API API;

	std::shared_ptr<Listener<WindowSizeChangedEvent>> sizeChange;
	std::shared_ptr<WindowManager> wm;
	std::shared_ptr<Scene> scene;
	std::shared_ptr<UIManager> ui;
	std::shared_ptr<gfx::Pipeline> pipeline;
	uint32 fps = 0;
	uint32 fpsAccumulator = 0;
	bool nominal = true;

	void onStart() override;

	void onTick() override;

	void onShutdown() override;
};