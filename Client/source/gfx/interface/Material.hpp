#pragma once
#include "Texture.hpp"
#include "Shader.hpp"

namespace gfx
{
	class Material
	{
	public:
		virtual std::shared_ptr<Shader> getShader() = 0;
		virtual std::vector<std::shared_ptr<Texture>> getTextures() = 0;
	};
};