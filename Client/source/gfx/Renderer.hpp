#pragma once
#include "interface/Modular.hpp"
#include "VkGfx.hpp"

class Renderer : public Modular, public Module
{
public:
	std::shared_ptr<WindowManager> wm;
	std::shared_ptr<VkGfx> gfx;

	void onStart() override
	{
		wm = getParent<Modular>()->getModule<WindowManager>("wm");
		gfx = initModule<VkGfx>("gfx");

		for (auto&& kv : modules)
		{
			kv.second->onStart();
		}
	};

	void onTick() override
	{
		if (wm->width > 0 && wm->height > 0)
		{
			if (wm->width != gfx->swapchain.extent.width || wm->height != gfx->swapchain.extent.height)
			{
				gfx->recreateSwapchain();
				return;
			}
			gfx->draw();
		}
	};

	void onShutdown() override
	{
		for (auto&& kv : modules)
		{
			kv.second->onShutdown();
		}
	};
};
