#include "WindowManager.hpp"
#include "log/Logger.hpp"
#include "glad/glad.h"
#include "../gfx/Renderer.hpp"
#include "../gfx/ogl/ContextOpenGL.hpp"

bool WindowManager::vsync = false;

void WindowManager::onStart()
{
	auto events = getParent<Module>()->getParent<Viper>()->getModule<Events>("events");
	closeEvent = events->initModule<EventHandler<WindowCloseRequestedEvent>>("wm_windowcloserequestedevent");
	sizeEvent = events->initModule<EventHandler<WindowSizeChangedEvent>>("wm_windowsizechangedevent");
	keyPressedEvent = events->initModule<EventHandler<KeyPressedEvent>>("wm_keypressedevent");
	keyReleaseEvent = events->initModule<EventHandler<KeyReleasedEvent>>("wm_keyreleasedevent");
	keyTypedEvent = events->initModule<EventHandler<KeyTypedEvent>>("wm_keytypedevent");
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

	if (!vsync)
	{
		glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
	}

	switch(Renderer::API)
	{
	default:
		context = std::make_shared<gfx::ContextOpenGL>();
		break;
	}

	context->init();

	glfwSetErrorCallback([](int code, const char* msg)
	{
		warn("GLFW error (%d): %s", code, msg);
	});

	context->setHandle(glfwCreateWindow(width, height, getParent<Module>()->getParent<Viper>()->getModule("game")->friendlyName.c_str(), nullptr, nullptr));
	glfwMakeContextCurrent((GLFWwindow*)context->handle);

	int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	if (!status) throw std::runtime_error("Failed to initialize GLAD");

	glfwSetWindowUserPointer((GLFWwindow*)context->handle, this);

	glfwSetWindowSizeCallback((GLFWwindow*)context->handle, [](GLFWwindow* win, int width, int height)
	{
		WindowManager* man = (WindowManager*)glfwGetWindowUserPointer(win);
		man->width = width;
		man->height = height;
		WindowSizeChangedEvent ev {  };
		ev.width = width;
		ev.height = height;
		man->sizeEvent->fire(ev);
	});

	glfwSetKeyCallback((GLFWwindow*)context->handle, [](GLFWwindow* win, int key, int scan, int action, int mods)
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

	glfwSetCharCallback((GLFWwindow*)context->handle, [](GLFWwindow* win, uint32 key)
	{
		WindowManager* man = (WindowManager*)glfwGetWindowUserPointer(win);
		KeyTypedEvent ev;
		ev.key = key;
		man->keyTypedEvent->fire(ev);
	});

	glfwSetMouseButtonCallback((GLFWwindow*)context->handle, [](GLFWwindow* win, int button, int action, int mods)
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

	glfwSetScrollCallback((GLFWwindow*)context->handle, [](GLFWwindow* win, double x, double y)
	{
		WindowManager* man = (WindowManager*)glfwGetWindowUserPointer(win);
		ScrollEvent ev;
		ev.x = x;
		ev.y = y;
		man->scrollEvent->fire(ev);
	});

	glfwSetCursorPosCallback((GLFWwindow*)context->handle, [](GLFWwindow* win, double x, double y)
	{
		WindowManager* man = (WindowManager*)glfwGetWindowUserPointer(win);
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

	if (glfwWindowShouldClose((GLFWwindow*)context->handle))
	{
		WindowCloseRequestedEvent close;
		closeEvent->fire(close, &closeFuture);
	}
};

void WindowManager::onTickEnd()
{
	context->swapBuffers(vsync);
	glfwPollEvents();
};

void WindowManager::setShowCursor(bool show)
{
	glfwSetInputMode((GLFWwindow*)context->handle, GLFW_CURSOR, show ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	if (show)
	{
		glfwSetCursorPos((GLFWwindow*)context->handle, (double)showCursorCoords.x, (double)showCursorCoords.y);
	}
	else
	{
		double x;
		double y;
		glfwGetCursorPos((GLFWwindow*)context->handle, &x, &y);
		showCursorCoords = vec2((float)x, (float)y);
	}
}

void WindowManager::onShutdown()
{
	glfwDestroyWindow((GLFWwindow*)context->handle);
	glfwTerminate();
};