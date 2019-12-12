#include "Renderer.hpp"
#include "imgui.h"
#include "../Client.hpp"
#include "../gfx/ogl/PipelineOpenGL.hpp"

std::vector<float> cubeVerts =
{
	// Back face
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right         
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
	// Front face
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
	// Left face
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
	// Right face
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right         
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left     
	// Bottom face
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
	 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
	// Top face
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right     
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f  // bottom-left        
};

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
		ImGui::Begin("Telemetry", &active, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Connected: %d", renderer->getParent<Client>()->isConnected.load());
		ImGui::Checkbox("Vsync", &renderer->wm->vsync);
		ImGui::Text("FPS: %d", renderer->fps);
		ImGui::Text("Frametime: %.4fms", renderer->dt);
		ImGui::Text("Scene tick: %.4fms", renderer->scene->updateTimeMs);
		ImGui::Text("Draws: %d", std::reinterpret_pointer_cast<gfx::PipelineOpenGL>(renderer->pipeline)->drawCalls);
		ImGui::Text("Entities: %d", renderer->scene->container->heap.size() / renderer->scene->container->entitySize);
		ImGui::End();
	}, { std::reinterpret_pointer_cast<Module>(shared_from_this()) });

	ui->addCommand("selection_control", [](std::vector<std::shared_ptr<Module>> mods)
	{
		auto renderer = std::reinterpret_pointer_cast<Renderer>(mods[0]);
		bool active = true;
		auto selected = renderer->scene->container->getEntity(1);
		ImGui::Begin("Selection", &active, ImGuiWindowFlags_AlwaysAutoResize);
		auto transform = renderer->scene->container->getComponent<Transform3D>(selected->id);
		auto renderData = renderer->scene->container->getComponent<RenderData>(selected->id);
		if (ImGui::InputFloat3("Position", glm::value_ptr(transform->position)))
		{
			renderData->dirty = true;
		}
		if (ImGui::InputFloat3("Rotation Axis", glm::value_ptr(transform->rotationAxis)))
		{
			renderData->dirty = true;
		}
		if (ImGui::SliderFloat("Rotation", &transform->rotation, -180.0f, 180.0f))
		{
			renderData->dirty = true;
		}
		if (ImGui::InputFloat3("Scale", glm::value_ptr(transform->scale)))
		{
			renderData->dirty = true;
		}
		ImGui::End();
	}, { std::reinterpret_pointer_cast<Module>(shared_from_this()) });

	ui->addCommand("camera_control", [](std::vector<std::shared_ptr<Module>> mods)
	{
		auto renderer = std::reinterpret_pointer_cast<Renderer>(mods[0]);
		bool active = true;
		auto selected = renderer->scene->container->getEntity(0);
		ImGui::Begin("Camera", &active, ImGuiWindowFlags_AlwaysAutoResize);
		auto transform = renderer->scene->container->getComponent<Transform3D>(selected->id);
		auto cam = renderer->scene->container->getComponent<PerspectiveCamera>(selected->id);
		if (ImGui::InputFloat3("Position", glm::value_ptr(transform->position)))
		{
			cam->dirty = true;
		}
		if (ImGui::InputFloat3("Offset", glm::value_ptr(cam->offset)))
		{
			cam->dirty = true;
		}
		if (ImGui::InputFloat3("Target", glm::value_ptr(cam->target)))
		{
			cam->dirty = true;
		}
		if (ImGui::InputFloat3("Up", glm::value_ptr(cam->upAxis)))
		{
			cam->dirty = true;
		}
		if (ImGui::InputFloat3("Front", glm::value_ptr(cam->frontAxis)))
		{
			cam->dirty = true;
		}
		ImGui::Text("Yaw: %.2f", cam->yaw);
		ImGui::Text("Pitch: %.2f", cam->pitch);
		ImGui::Text("Roll: %.2f", cam->roll);
		ImGui::Text("Zoom: %.2f", cam->zoom);
		ImGui::End();
	}, { std::reinterpret_pointer_cast<Module>(shared_from_this()) });

	std::vector<float> vertices =
	{
		 -0.5f, -0.5f,
		  0.5f, -0.5f,
		  0.5f,  0.5f,
		 -0.5f,  0.5f
	};

	std::vector<float> verticesTex =
	{
		 -0.5f, -0.5f, 0.0f, 0.0f,
		  0.5f, -0.5f, 1.0f, 0.0f,
		  0.5f,  0.5f, 1.0f, 1.0f,
		 -0.5f,  0.5f, 0.0f, 1.0f
	};

	std::vector<uint32> indices =
	{
		0, 1, 3,
		1, 2, 3
	};

	pipeline->getMemory()->requestBuffer("cube", &cubeVerts, nullptr, { {gfx::Float3, "position" }, { gfx::Float2, "texCoord" } }, { { gfx::Float4x4, "model", false, 1 } });
	pipeline->getMemory()->requestBuffer("cube2", &cubeVerts, nullptr, { {gfx::Float3, "position" }, { gfx::Float2, "texCoord" } }, { { gfx::Float4x4, "model", false, 1 } });

	pipeline->loadShader("3d_default");
	pipeline->loadShader("3d_default2");

	pipeline->loadTexture("checkerboard.png");
	pipeline->loadTexture("awesomeface.png");
	
	pipeline->makeMaterial("3d_default", "3d_default", { "checkerboard.png" });
	pipeline->makeMaterial("3d_default2", "3d_default2", { "awesomeface.png" });
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
		pipeline->submit(kv.second);
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