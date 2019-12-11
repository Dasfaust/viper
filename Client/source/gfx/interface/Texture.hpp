#pragma once
#include "Defines.hpp"

namespace gfx
{
	class Texture
	{
	public:
		virtual uint32 getWidth() = 0;
		virtual uint32 getHeight() = 0;

		virtual void bind(uint32 slot) = 0;
		virtual void cleanup() = 0;
	};

	class Texture2D : public Texture
	{
	public:
		virtual void init(const std::string& fileName) = 0;
	};
};