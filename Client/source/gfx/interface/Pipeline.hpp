#pragma once
#include "Defines.hpp"
#include "Memory.hpp"
#include "Shader.hpp"
#include "../Scene.hpp"
#include "Texture.hpp"
#include "Material.hpp"

namespace gfx
{
	class Pipeline : public Module
	{
	public:
		virtual bool init() = 0;
		virtual void setClearColor(vec4 color) = 0;
		virtual void setViewport(vec2 size, vec2 offset = { 0.0f, 0.0f }) = 0;
		virtual std::shared_ptr<Memory> getMemory() = 0;
		virtual std::shared_ptr<Shader> loadShader(const std::string& name) = 0;
		virtual std::shared_ptr<Shader> getShader(const std::string& name) = 0;
		virtual std::shared_ptr<Texture> loadTexture(const std::string& name) = 0;
		virtual std::shared_ptr<Texture> getTexture(const std::string& name) = 0;
		virtual std::shared_ptr<Material> makeMaterial(const std::string& name, const std::string& shaderName, const std::vector<std::string>& textureNames) = 0;
		virtual std::shared_ptr<Material> getMaterial(const std::string& name) = 0;
		virtual std::shared_ptr<Material> getMaterial(uint32 id) = 0;
		virtual void submit(InstanceMap& instances) = 0;
		virtual void draw() = 0;
	};
};