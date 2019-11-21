#pragma once
#include "interface/Modular.hpp"
#include "Scene.hpp"
#include "../surface/WindowManager.hpp"
#include "../ui/UIManager.hpp"

class Renderer : public Modular, public Module
{
public:
	std::shared_ptr<Listener<WindowSizeChangedEvent>> sizeChange;

	std::shared_ptr<WindowManager> wm;
	std::shared_ptr<Scene> scene;
	std::shared_ptr<UIManager> ui;
	uint32 fps = 0;
	uint32 fpsAccumulator = 0;

	void onStart() override;

	void onTick() override;

	void onShutdown() override;
};