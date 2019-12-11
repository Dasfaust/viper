#pragma once
#include "Defines.hpp"

namespace gfx
{
	class Shader
	{
	public:
		uint32 id;

		virtual void init(const std::string& fileName, bool source = false) = 0;
		virtual void compile() = 0;
		virtual void cleanup() = 0;
		virtual void bind() = 0;
		virtual void unbind() = 0;
	};
};