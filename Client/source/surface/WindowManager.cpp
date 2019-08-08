#include "WindowManager.hpp"
#include "log/Logger.hpp"

void WindowManager::onStart()
{
	auto events = getParent<Module>()->getParent<Viper>()->getModule<Events>("events");
	closeEvent = events->initModule<EventHandler<WindowCloseRequestedEvent>>("wm_windowcloserequestedevent");
	sizeEvent = events->initModule<EventHandler<WindowSizeChangedEvent>>("wm_windowsizechangedevent");
	keyPressedEvent = events->initModule<EventHandler<KeyPressedEvent>>("wm_keypressedevent");
	keyReleaseEvent = events->initModule<EventHandler<KeyReleasedEvent>>("wm_keyreleasedevent");
	buttonPressedEvent = events->initModule<EventHandler<ButtonPressedEvent>>("wm_buttonpressevent");
	buttonReleasedEvent = events->initModule<EventHandler<ButtonReleasedEvent>>("wm_buttonreleasedevent");
	scrollEvent = events->initModule<EventHandler<ScrollEvent>>("wm_scrollevent");
	mouseMoveEvent = events->initModule<EventHandler<MouseMoveEvent>>("wm_mousemoveevent");

	int ok = glfwInit();
	if (!ok)
	{
		crit("GLFW returned error code %d", ok);
		return;
	}

	glfwSetErrorCallback([](int code, const char* msg)
	{
		warn("GLFW error (%d): %s", code, msg);
	});

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(width, height, getParent<Module>()->getParent<Viper>()->getModule("game")->friendlyName.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);

	glfwSetWindowSizeCallback(window, [](GLFWwindow* win, int width, int height)
	{
		WindowManager* man = (WindowManager*)glfwGetWindowUserPointer(win);
		man->width = width;
		man->height = height;
		WindowSizeChangedEvent ev;
		ev.width = width;
		ev.height = height;
		man->sizeEvent->fire(ev);
	});

	glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int scan, int action, int mods)
	{
		WindowManager* man = (WindowManager*)glfwGetWindowUserPointer(win);
		switch(action)
		{
			case GLFW_PRESS:
			{
				KeyPressedEvent ev;
				ev.key = key;
				ev.mods = mods;
				ev.repeat = 0;
				man->keyPressedEvent->fire(ev);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent ev;
				ev.key = key;
				ev.mods = mods;
				man->keyReleaseEvent->fire(ev);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyPressedEvent ev;
				ev.key = key;
				ev.mods = mods;
				ev.repeat = 1;
				man->keyPressedEvent->fire(ev);
				break;
			}
		}
	});

	glfwSetMouseButtonCallback(window, [](GLFWwindow* win, int button, int action, int mods)
	{
		WindowManager* man = (WindowManager*)glfwGetWindowUserPointer(win);
		switch (action)
		{
			case GLFW_PRESS:
			{
				ButtonPressedEvent ev;
				ev.button = button;
				ev.mods = mods;
				man->buttonPressedEvent->fire(ev);
				break;
			}
			case GLFW_RELEASE:
			{
				ButtonReleasedEvent ev;
				ev.button = button;
				ev.mods = mods;
				man->buttonReleasedEvent->fire(ev);
				break;
			}
		}
	});

	glfwSetScrollCallback(window, [](GLFWwindow* win, double x, double y)
	{
		WindowManager* man = (WindowManager*)glfwGetWindowUserPointer(win);
		man->scrollX = x;
		man->scrollY = y;
		ScrollEvent ev;
		ev.x = x;
		ev.y = y;
		man->scrollEvent->fire(ev);
	});

	glfwSetCursorPosCallback(window, [](GLFWwindow* win, double x, double y)
	{
		WindowManager* man = (WindowManager*)glfwGetWindowUserPointer(win);
		man->mouseX = x;
		man->mouseY = y;
		MouseMoveEvent ev;
		ev.x = x;
		ev.y = y;
		man->mouseMoveEvent->fire(ev);
	});
};

void WindowManager::onTickBegin()
{
	WindowCloseRequestedEvent ev;
	if (closeFuture.tryGet(ev))
	{
		if (!ev.cancelled)
		{
			Viper::shutdown();
		}
	}

	if (glfwWindowShouldClose(window))
	{
		WindowCloseRequestedEvent close;
		closeEvent->fire(close, &closeFuture);
	}

	glfwPollEvents();
};

void WindowManager::onShutdown()
{
	glfwDestroyWindow(window);
	glfwTerminate();
};