#include "Renderer.hpp"
#include "imgui.h"
#include "../Client.hpp"
#include "../gfx/ogl/PipelineOpenGL.hpp"

GFX_API Renderer::API = OPEN_GL;

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

	switch(API)
	{
	default:
		pipeline = initModule<gfx::PipelineOpenGL>("pipeline");
		break;
	}
	if (!pipeline->init())
	{
		crit("Pipeline could not be initialized, can't continue");
		nominal = false;
		return;
	}
	pipeline->setClearColor({ 0.2f, 0.2f, 0.2f, 1.0f });
	pipeline->setViewport({ wm->width, wm->height });

	initModule<FPSModule>("fps", 1000.0);
	ui = initModule<UIManager>("ui");
	scene = initModule<Scene>("scene");

	for (auto&& kv : modules)
	{
		kv.second->onStart();
	}

	sizeChange = wm->sizeEvent->listen(0, [](WindowSizeChangedEvent& ev, std::vector<std::shared_ptr<Module>> mods)
	{
		std::reinterpret_pointer_cast<gfx::Pipeline>(mods[0])->setViewport({ ev.width, ev.height });
	}, { pipeline });

	ui->addCommand("client_telemetry", [](std::vector<std::shared_ptr<Module>> mods)
	{
		auto renderer = std::reinterpret_pointer_cast<Renderer>(mods[0]);
		bool active = true;
		auto pos = ImGui::GetMainViewport()->Pos;
		ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y));
		ImGui::SetNextWindowViewport(ImGui::FindViewportByPlatformHandle(renderer->wm->context->handle)->ID);
		ImGui::Begin("Client", &active, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
		//ImGui::Text("Vsync: %d", renderer->wm->vsync);
		ImGui::Text("Connected: %d", renderer->getParent<Client>()->isConnected.load());
		ImGui::Checkbox("Vsync", &renderer->wm->vsync);
		ImGui::Text("FPS: %d", renderer->fps);
		ImGui::Text("Frametime: %.4fms", renderer->dt);
		ImGui::Text("Entities: %d", renderer->scene->container->heap.size() / renderer->scene->container->entitySize);
		ImGui::Text("Scene tick: %.4fms", renderer->scene->updateTimeMs);
		ImGui::Text("Draws: %d", std::reinterpret_pointer_cast<gfx::PipelineOpenGL>(renderer->pipeline)->drawCalls);
		ImGui::End();
	}, { std::reinterpret_pointer_cast<Module>(shared_from_this()) });

	std::vector<vec2> vertices =
	{
		 vec2(-0.5f,  -0.5f),
		 vec2(0.5f, -0.5f),
		 vec2(0.5f, 0.5f),
		 vec2(-0.5f,  0.5f)
	};

	std::vector<vec2> verticesTex =
	{
		 vec2(-0.5f,  -0.5f), vec2(0.0f, 0.0f),
		 vec2(0.5f, -0.5f), vec2(1.0f, 0.0f),
		 vec2(0.5f, 0.5f), vec2(1.0f, 1.0f),
		 vec2(-0.5f,  0.5f), vec2(0.0f, 1.0f)
	};

	std::vector<uint32> indices =
	{
		0, 1, 3,
		1, 2, 3
	};
	pipeline->getMemory()->requestBuffer("plane", vertices, indices, { {gfx::Float2, "position" } }, { { gfx::Float4x4, "model", false, 1 } });
	pipeline->getMemory()->requestBuffer("plane_texture", verticesTex, indices, { {gfx::Float2, "position" }, { gfx::Float2, "texCoord" } }, { { gfx::Float4x4, "model", false, 1 } });
	pipeline->loadShader("2d_basic");
	pipeline->loadShader("2d_basic_texture");

	pipeline->loadTexture("checkerboard.png");
	pipeline->loadTexture("logo.png");
};

void Renderer::onTick()
{
	if (!nominal) return;

	sizeChange->poll();

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

	for (auto&& kv : scene->renderSystem->renderData)
	{
		std::string shaderName = kv.first;
		for (auto&& mkv : kv.second)
		{
			std::string meshName = mkv.first;
			pipeline->submit(shaderName, meshName, mkv.second);
		}
	}

	pipeline->draw();

	for (auto&& kv : modules)
	{
		kv.second->onTickEnd();
	}

	dt = timesince(lastTickNs);
	lastTickNs = tnowns();
	fpsAccumulator++;
};

void Renderer::onShutdown()
{
	for (auto&& kv : modules)
	{
		kv.second->onShutdown();
	}
};