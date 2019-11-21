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

	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
	io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	
	if (Renderer::API == OPEN_GL)
	{
		ImGui_ImplOpenGL3_Init("#version 410");
		ImGui_ImplGlfw_InitForOpenGL(wm->window, true);
	}
};

void UIManager::onTickBegin()
{

};

void UIManager::onTickEnd()
{
	ImGuiIO& io = ImGui::GetIO();
	time_val delta = (float)getParent<Modular>()->dt;
	time_val maxDt = 1.0 / 240.0;
	io.DeltaTime = delta > maxDt ? (float)maxDt : (float)delta;
	io.DisplaySize = ImVec2((float)wm->width, (float)wm->height);

	if (Renderer::API == OPEN_GL)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
	}
	
	ImGui::NewFrame();

	for (auto&& kv : commands)
	{
		kv.second.func(kv.second.modules);
	}

	ImGui::Render();

	if (Renderer::API == OPEN_GL)
	{
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
	if (Renderer::API == OPEN_GL)
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
	}
};