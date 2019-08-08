#pragma once
#include "../surface/WindowManager.hpp"
#include "KeyCodes.hpp"
#include <glm/vec2.hpp>

struct KeyState
{
	time_val when = 0.0;
	uint32 repeats = 0;
	time_val elapsed = 0.0;
};

class InputManager : public Module
{
public:
	std::shared_ptr<WindowManager> wm;
	std::shared_ptr<Listener<ButtonPressedEvent>> buttonPressed;
	std::shared_ptr<Listener<ButtonReleasedEvent>> buttonReleased;
	std::shared_ptr<Listener<KeyPressedEvent>> keyPressed;
	std::shared_ptr<Listener<KeyReleasedEvent>> keyReleased;
	std::shared_ptr<Listener<ScrollEvent>> scroll;
	std::shared_ptr<Listener<MouseMoveEvent>> move;
	flatmap(uint32, KeyState) keys;
	glm::vec2 mousePos;
	glm::vec2 scrollPos;

	void onStart() override
	{
		wm = getParent<Modular>()->getModule<WindowManager>("wm");

		buttonPressed = wm->buttonPressedEvent->listen(0, [](ButtonPressedEvent& ev, std::vector<std::shared_ptr<Module>> mods)
		{
			auto in = std::dynamic_pointer_cast<InputManager>(mods[0]);
			KeyState state;
			if (in->keys.count(ev.button))
			{
				state = in->keys[ev.button];
				state.repeats++;
			}
			state.when = tnowns();
			in->keys[ev.button] = state;
		}, { getParent<Modular>()->getModule("input") });

		buttonReleased = wm->buttonReleasedEvent->listen(0, [](ButtonReleasedEvent& ev, std::vector<std::shared_ptr<Module>> mods)
		{
			auto in = std::dynamic_pointer_cast<InputManager>(mods[0]);
			if (in->keys.count(ev.button))
			{
				in->keys.erase(ev.button);
			}
		}, { getParent<Modular>()->getModule("input") });

		keyPressed = wm->keyPressedEvent->listen(0, [](KeyPressedEvent& ev, std::vector<std::shared_ptr<Module>> mods)
		{
			auto in = std::dynamic_pointer_cast<InputManager>(mods[0]);
			KeyState state;
			if (in->keys.count(ev.key))
			{
				state = in->keys[ev.key];
				state.repeats++;
			}
			state.when = tnowns();
			in->keys[ev.key] = state;
		}, { getParent<Modular>()->getModule("input") });

		keyReleased = wm->keyReleaseEvent->listen(0, [](KeyReleasedEvent& ev, std::vector<std::shared_ptr<Module>> mods)
		{
			auto in = std::dynamic_pointer_cast<InputManager>(mods[0]);
			if (in->keys.count(ev.key))
			{
				in->keys.erase(ev.key);
			}
		}, { getParent<Modular>()->getModule("input") });

		scroll = wm->scrollEvent->listen(0, [](ScrollEvent& ev, std::vector<std::shared_ptr<Module>> mods)
		{
			auto in = std::dynamic_pointer_cast<InputManager>(mods[0]);
			in->scrollPos.x = (float)ev.x;
			in->scrollPos.y = (float)ev.y;
		}, { getParent<Modular>()->getModule("input") });

		move = wm->mouseMoveEvent->listen(0, [](MouseMoveEvent& ev, std::vector<std::shared_ptr<Module>> mods)
		{
			auto in = std::dynamic_pointer_cast<InputManager>(mods[0]);
			in->mousePos.x = (float)ev.x;
			in->mousePos.y = (float)ev.y;
		}, { getParent<Modular>()->getModule("input") });
	};

	void onTick() override
	{
		buttonPressed->poll();
		buttonReleased->poll();
		keyPressed->poll();
		keyReleased->poll();
		scroll->poll();
		move->poll();

		for (auto&& kv : keys)
		{
			kv.second.elapsed = timesince(kv.second.when);
		}
	};

	bool isDown(uint32 key)
	{
		return keys.count(key);
	};

	bool isDown(uint32 key, KeyState& state)
	{
		if (keys.count(key))
		{
			state = keys[key];
			return true;
		}
		return false;
	};
};