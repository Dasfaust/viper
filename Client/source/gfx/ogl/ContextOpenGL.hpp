#pragma once
#include "../interface/Context.hpp"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "log/Logger.hpp"

namespace gfx
{
	class ContextOpenGL : public Context
	{
	public:
		void init() override
		{
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		};

		void swapBuffers(bool vsync) override
		{
			if (vsync)
			{
				glfwSwapBuffers((GLFWwindow*)handle);
			}
			else
			{
				glFlush();
			}
		};
	};
};