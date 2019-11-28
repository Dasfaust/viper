#pragma once

namespace gfx
{
	class Context
	{
	public:
		void* handle;

		void setHandle(void* handle)
		{
			this->handle = handle;
		};

		virtual void init() = 0;
		virtual void swapBuffers(bool vsync) = 0;
	};
};