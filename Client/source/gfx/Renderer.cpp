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
		ImGui::Begin("Telemetry", &active, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::CollapsingHeader("Client");
		ImGui::Text("Connected: %d", renderer->getParent<Client>()->isConnected.load());
		ImGui::Checkbox("Vsync", &renderer->wm->vsync);
		ImGui::Text("FPS: %d", renderer->fps);
		ImGui::Text("Frametime: %.4fms", renderer->deltaTimeMs);
		ImGui::Text("Scene tick: %.4fms", renderer->scene->tickTimeMs);
		ImGui::Text("Draws: %d", std::reinterpret_pointer_cast<gfx::PipelineOpenGL>(renderer->pipeline)->drawCalls);
		ImGui::Text("Entities: %d", renderer->scene->container->heap.size() / renderer->scene->container->entitySize);
		auto client = renderer->getParent<Client>();
		if (client->enableNetworking)
		{
			ImGui::CollapsingHeader("Server");
			ImGui::Text("Server delta: %.2fms", client->serverTelemetry.serverDelta);
			ImGui::Text("Server tick: %.2fms", client->serverTelemetry.serverTick);
			ImGui::Text("World delta: %.2fms", client->serverTelemetry.worldDelta);
			ImGui::Text("World tick: %.2fms", client->serverTelemetry.worldTick);
			ImGui::Text("World TPS: %d", client->serverTelemetry.worldTps);
			ImGui::CollapsingHeader("Network");
			ImGui::Text("Ping: %.2fms", client->serverTelemetry.ping);
			auto net = client->getModule<NetClient>("net");
			ImGui::Text("Client thread delta: %.2fms", Time::toMilliseconds(net->ip->deltaTime));
			ImGui::Text("Client thread tick: %.2fms", Time::toMilliseconds(net->ip->tickTime));
			ImGui::Text("Client thread incoming: %.2fms", Time::toMilliseconds(net->ip->incomingTime));
			ImGui::Text("Client thread outgoing: %.2fms", Time::toMilliseconds(net->ip->outgoingTime));
			ImGui::Text("Client processing: %.2fms", net->tickTimeMs);
			ImGui::Text("Server thread delta: %.2fms", client->serverTelemetry.serverIpDelta);
			ImGui::Text("Server thread tick: %.2fms", client->serverTelemetry.serverIpTick);
			ImGui::Text("Server thread incoming: %.2fms", client->serverTelemetry.serverIpIncoming);
			ImGui::Text("Server thread outgoing: %.2fms", client->serverTelemetry.serverIpOutgoing);
			ImGui::Text("Server processing: %.2fms", client->serverTelemetry.serverNsTick);
			ImGui::Text("Total processing: %.2fms", Time::toMilliseconds(net->ip->tickTime) + net->tickTimeMs + client->serverTelemetry.serverIpTick + client->serverTelemetry.serverNsTick);
		}
		ImGui::End();
	}, { std::reinterpret_pointer_cast<Module>(shared_from_this()) });

	ui->addCommand("selection_control", [](std::vector<std::shared_ptr<Module>> mods)
	{
		auto renderer = std::reinterpret_pointer_cast<Renderer>(mods[0]);
		bool active = true;
		auto selected = renderer->scene->container->getEntity(0);
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

	pipeline->loadTexture("checkerboard.png");
	pipeline->loadTexture("awesomeface.png");
	
	pipeline->makeMaterial("3d_default", "3d_default", { "checkerboard.png" });
	pipeline->makeMaterial("3d_default2", "3d_default", { "awesomeface.png" });
};

void Renderer::onTick()
{
	if (!nominal) return;


	auto start = Time::now();
	deltaTime = Time::since(start, lastTick);
	lastTick = start;
	deltaTimeMs = Time::toMilliseconds(deltaTime);
	deltaTimeS = Time::toSeconds(deltaTime);

	sizeChange->poll();
	
	for (auto&& kv : modules)
	{
		kv.second->onTickBegin();
	}

	for (auto&& kv : modules)
	{
		if (kv.second->modInterval > 0.0f)
		{
			if (Time::toMilliseconds(kv.second->modAccumulator) >= kv.second->modInterval)
			{
				kv.second->modAccumulator = 0;
				kv.second->onTick();
			}
			else
			{
				kv.second->modAccumulator += deltaTime;
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

	tickTimeMs = Time::toMilliseconds(Time::since(start));

	fpsAccumulator++;
};

void Renderer::onShutdown()
{
	for (auto&& kv : modules)
	{
		kv.second->onShutdown();
	}
};