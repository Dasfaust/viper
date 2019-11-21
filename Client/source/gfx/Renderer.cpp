#include "Renderer.hpp"

void FPSReporter::onTick()
{
	auto renderer = getParent<Renderer>();
	debug("FPS: %d", renderer->fps);
	renderer->fps = 0;
};

void Renderer::onStart()
{
	wm = getParent<Modular>()->getModule<WindowManager>("wm");
	//initModule<FPSReporter>("fps", 1000.0);
	scene = initModule<Scene>("scene");

	for (auto&& kv : modules)
	{
		kv.second->onStart();
	}
};

void Renderer::onTick()
{
	tickModules();
};

void Renderer::onShutdown()
{
	for (auto&& kv : modules)
	{
		kv.second->onShutdown();
	}
};