#include "Renderer.hpp"
#include "glad/glad.h"

class FPSModule : public Module
{
	void onTick() override
	{
		auto renderer = getParent<Renderer>();
		renderer->fps = renderer->fpsAccumulator;
		renderer->fpsAccumulator = 0;
	};
};

void Renderer::onStart()
{
	wm = getParent<Modular>()->getModule<WindowManager>("wm");

	sizeChange = wm->sizeEvent->listen(0, [](WindowSizeChangedEvent& ev, std::vector<std::shared_ptr<Module>> mods)
	{
		glViewport(0, 0, (int)ev.height, (int)ev.width);
	});

	initModule<FPSModule>("fps", 1000.0);
	ui = initModule<UIManager>("ui");
	scene = initModule<Scene>("scene");

	for (auto&& kv : modules)
	{
		kv.second->onStart();
	}

	glViewport(0, 0, (int)wm->height, (int)wm->width);
};

void Renderer::onTick()
{
	sizeChange->poll();

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	for (auto&& kv : modules)
	{
		kv.second->onTickBegin();
	}

	for (auto&& kv : modules)
	{
		if (kv.second->modInterval > 0)
		{
			if (kv.second->modAccumulator >= kv.second->modInterval)
			{
				kv.second->modAccumulator = 0.0;
				kv.second->onTick();
			}
			else
			{
				kv.second->modAccumulator += dt;
				kv.second->onTickWait();
			}
		}
		else
		{
			kv.second->onTick();
		}
	}

	for (auto&& kv : modules)
	{
		kv.second->onTickEnd();
	}

	dt = timesince(lastTickNs);
	lastTickNs = tnowns();
};

void Renderer::onShutdown()
{
	for (auto&& kv : modules)
	{
		kv.second->onShutdown();
	}
};