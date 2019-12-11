#pragma once
#include "../interface/Material.hpp"
#include "TextureOpenGL.hpp"
#include "ShaderOpenGL.hpp"

namespace gfx
{
	class MaterialOpenGL : public Material
	{
	public:
		std::shared_ptr<ShaderOpenGL> shader;
		std::vector<std::shared_ptr<Texture2DOpenGL>> textures;

		std::shared_ptr<Shader> getShader() override
		{
			return shader;
		};

		std::vector<std::shared_ptr<Texture>> getTextures() override
		{
			return (*(std::vector<std::shared_ptr<Texture>>*)&textures);
		};
	};
};