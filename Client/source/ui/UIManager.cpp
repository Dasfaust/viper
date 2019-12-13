#include "UIManager.hpp"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "examples/imgui_impl_opengl3.h"
#include "examples/imgui_impl_glfw.h"
#include "../gfx/Renderer.hpp"

void UIManager::onStart()
{
	wm = getParent<Module>()->getParent<Modular>()->getModule<WindowManager>("wm");

	buttonPressed = wm->buttonPressedEvent->listen(5, [](ButtonPressedEvent& ev, std::vector<std::shared_ptr<Module>> mods)
	{
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse)
		{
			ev.cancelled = true;
		}
	}, { shared_from_this() });

	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)wm->width, (float)wm->height);
	io.WantSaveIniSettings = false;
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
	io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, 0.0f);

	switch (Renderer::API)
	{
	default:
		ImGui_ImplOpenGL3_Init("#version 410");
		ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)wm->context->handle, true);
		break;
	}
};

void UIManager::onTickBegin()
{
	buttonPressed->poll();
};

void UIManager::onTickEnd()
{
	ImGuiIO& io = ImGui::GetIO();
	float delta = getParent<Modular>()->deltaTimeMs;
	float maxDt = 1.0f / 240.0f;
	io.DeltaTime = delta > maxDt ? maxDt : delta;
	io.DisplaySize = ImVec2((float)wm->width, (float)wm->height);

	switch (Renderer::API)
	{
	default:
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
	}
	
	ImGui::NewFrame();

	for (auto&& kv : commands)
	{
		kv.second.func(kv.second.modules);
	}

	ImGui::Render();

	switch (Renderer::API)
	{
	default:
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup);
	}
};

void UIManager::onShutdown()
{
	switch (Renderer::API)
	{
	default:
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
	}
};