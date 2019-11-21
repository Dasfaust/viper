#pragma once
#include "Viper.hpp"
#include "event/Events.hpp"

struct GLFWwindow;

struct WindowCloseRequestedEvent : Event
{ };

struct WindowSizeChangedEvent : Event
{
	int width;
	int height;
};

struct KeyEvent : Event
{
	int key;
	int mods;
};
struct KeyPressedEvent : KeyEvent
{
	int repeat;
};
struct KeyReleasedEvent : KeyEvent
{ };

struct KeyTypedEvent : Event
{
	uint32 key;
};

struct ButtonEvent : Event
{
	int button;
	int mods;
};
struct ButtonPressedEvent : ButtonEvent
{ };
struct ButtonReleasedEvent : ButtonEvent
{ };

struct ScrollEvent : Event
{
	double x;
	double y;
};

struct MouseMoveEvent : Event
{
	double x;
	double y;
};

class WindowManager : public Modular, public Module
{
public:
	int width = 1280;
	int height = 720;
	GLFWwindow* window;
	std::shared_ptr<EventHandler<WindowCloseRequestedEvent>> closeEvent;
	Future<WindowCloseRequestedEvent> closeFuture;
	std::shared_ptr<EventHandler<WindowSizeChangedEvent>> sizeEvent;
	std::shared_ptr<EventHandler<KeyPressedEvent>> keyPressedEvent;
	std::shared_ptr<EventHandler<KeyReleasedEvent>> keyReleaseEvent;
	std::shared_ptr<EventHandler<KeyTypedEvent>> keyTypedEvent;
	std::shared_ptr<EventHandler<ButtonPressedEvent>> buttonPressedEvent;
	std::shared_ptr<EventHandler<ButtonReleasedEvent>> buttonReleasedEvent;
	std::shared_ptr<EventHandler<ScrollEvent>> scrollEvent;
	std::shared_ptr<EventHandler<MouseMoveEvent>> mouseMoveEvent;

	void onStart() override;

	void onTickBegin() override;

	void onTickEnd() override;

	void onShutdown() override;
};