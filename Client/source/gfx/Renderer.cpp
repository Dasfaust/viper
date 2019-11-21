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
	gfx = initModule<VkGfx>("gfx");
	//initModule<FPSReporter>("fps", 1000.0);
	scene = initModule<Scene>("scene");

	for (auto&& kv : modules)
	{
		kv.second->onStart();
	}
};

void Renderer::onTick()
{
	if (gfx->nominal)
	{
		if (wm->width > 0 && wm->height > 0)
		{
			if (wm->width != gfx->swapchain.extent.width || wm->height != gfx->swapchain.extent.height)
			{
				gfx->recreateSwapchain();
				return;
			}
			gfx->draw();
			fps++;
		}
	}
	tickModules();
};

void Renderer::onShutdown()
{
	for (auto&& kv : modules)
	{
		kv.second->onShutdown();
	}
};