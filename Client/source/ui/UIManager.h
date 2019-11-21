#pragma once
#include "interface/Module.hpp"
#include "../surface/WindowManager.hpp"
#include "imgui.h"
#include "../gfx/ogl/imgui_impl_opengl3.h"
#include "../input/InputManager.hpp"

class UIManager : public Module
{
public:
	std::shared_ptr<WindowManager> wm;
	std::shared_ptr<InputManager> input;

	std::shared_ptr<Listener<ButtonPressedEvent>> buttonPressed;
	std::shared_ptr<Listener<ButtonReleasedEvent>> buttonReleased;
	std::shared_ptr<Listener<MouseMoveEvent>> mouseMoved;
	std::shared_ptr<Listener<ScrollEvent>> mouseScrolled;
	std::shared_ptr<Listener<KeyPressedEvent>> keyPressed;
	std::shared_ptr<Listener<KeyReleasedEvent>> keyReleased;
	std::shared_ptr<Listener<KeyTypedEvent>> keyTyped;

	void onStart() override
	{
		wm = getParent<Module>()->getParent<Modular>()->getModule<WindowManager>("wm");
		input = getParent<Module>()->getParent<Modular>()->getModule<InputManager>("input");

		buttonPressed = wm->buttonPressedEvent->listen(10, [](ButtonPressedEvent& ev, std::vector<std::shared_ptr<Module>> mods)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.MouseDown[ev.button] = true;
		});
		buttonReleased = wm->buttonReleasedEvent->listen(10, [](ButtonReleasedEvent& ev, std::vector<std::shared_ptr<Module>> mods)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.MouseDown[ev.button] = false;
		});
		mouseMoved = wm->mouseMoveEvent->listen(10, [](MouseMoveEvent& ev, std::vector<std::shared_ptr<Module>> mods)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.MousePos = ImVec2((float)ev.x, (float)ev.y);
		});
		mouseScrolled = wm->scrollEvent->listen(10, [](ScrollEvent& ev, std::vector<std::shared_ptr<Module>> mods)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.MouseWheelH += ev.x;
			io.MouseWheel += ev.y;
		});
		keyPressed = wm->keyPressedEvent->listen(10, [](KeyPressedEvent& ev, std::vector<std::shared_ptr<Module>> mods)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.KeysDown[ev.key] = true;
			io.KeyCtrl = io.KeysDown[KEY_LEFT_CONTROL] || io.KeysDown[KEY_RIGHT_CONTROL];
			io.KeyShift = io.KeysDown[KEY_LEFT_SHIFT] || io.KeysDown[KEY_RIGHT_SHIFT];
			io.KeyAlt = io.KeysDown[KEY_LEFT_ALT] || io.KeysDown[KEY_RIGHT_ALT];
			io.KeySuper = io.KeysDown[KEY_LEFT_SUPER] || io.KeysDown[KEY_RIGHT_SUPER];
		});
		keyReleased = wm->keyReleaseEvent->listen(10, [](KeyReleasedEvent& ev, std::vector<std::shared_ptr<Module>> mods)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.KeysDown[ev.key] = false;
			io.KeyCtrl = io.KeysDown[KEY_LEFT_CONTROL] || io.KeysDown[KEY_RIGHT_CONTROL];
			io.KeyShift = io.KeysDown[KEY_LEFT_SHIFT] || io.KeysDown[KEY_RIGHT_SHIFT];
			io.KeyAlt = io.KeysDown[KEY_LEFT_ALT] || io.KeysDown[KEY_RIGHT_ALT];
			io.KeySuper = io.KeysDown[KEY_LEFT_SUPER] || io.KeysDown[KEY_RIGHT_SUPER];
		});
		keyTyped = wm->keyTypedEvent->listen(10, [](KeyTypedEvent& ev, std::vector<std::shared_ptr<Module>> mods)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (ev.key > 0  && ev.key < 0x10000)
			{
				io.AddInputCharacter((unsigned short)ev.key);
			}
		});

		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

		io.KeyMap[ImGuiKey_Tab] = KEY_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = KEY_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = KEY_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = KEY_UP;
		io.KeyMap[ImGuiKey_DownArrow] = KEY_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = KEY_PAGE_UP;
		io.KeyMap[ImGuiKey_PageDown] = KEY_PAGE_DOWN;
		io.KeyMap[ImGuiKey_Home] = KEY_HOME;
		io.KeyMap[ImGuiKey_End] = KEY_END;
		io.KeyMap[ImGuiKey_Insert] = KEY_INSERT;
		io.KeyMap[ImGuiKey_Delete] = KEY_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = KEY_BACKSPACE;
		io.KeyMap[ImGuiKey_Space] = KEY_SPACE;
		io.KeyMap[ImGuiKey_Enter] = KEY_ENTER;
		io.KeyMap[ImGuiKey_Escape] = KEY_ESCAPE;
		io.KeyMap[ImGuiKey_A] = KEY_A;
		io.KeyMap[ImGuiKey_C] = KEY_C;
		io.KeyMap[ImGuiKey_V] = KEY_V;
		io.KeyMap[ImGuiKey_X] = KEY_X;
		io.KeyMap[ImGuiKey_Y] = KEY_Y;
		io.KeyMap[ImGuiKey_Z] = KEY_Z;

		ImGui_ImplOpenGL3_Init("#version 410");
	};

	void onTickBegin() override
	{
		buttonPressed->poll();
		buttonReleased->poll();
		mouseMoved->poll();
		mouseScrolled->poll();
		keyPressed->poll();
		keyReleased->poll();
		keyTyped->poll();
	};

	void onTickEnd() override
	{
		ImGuiIO& io = ImGui::GetIO();
		float delta = (float)getParent<Modular>()->dt;
		io.DeltaTime = delta > (1.0f / 144.0f) ? (1.0f / 144.0f) : delta;
		io.DisplaySize = ImVec2((float)wm->width, (float)wm->height);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		static bool show = true;
		ImGui::ShowDemoWindow(&show);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	};

	void onShutdown() override
	{

	};
};