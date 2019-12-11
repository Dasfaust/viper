#pragma once
#include "../interface/Pipeline.hpp"
#include "glad/glad.h"
#include "ShaderOpenGL.hpp"
#include "MemoryOpenGL.hpp"
#include "TextureOpenGL.hpp"
#include "MaterialOpenGL.hpp"

namespace gfx
{
	class PipelineOpenGL : public Pipeline
	{
	public:
		umap(std::string, std::shared_ptr<ShaderOpenGL>) shaders;
		umap(std::string, std::shared_ptr<Texture2DOpenGL>) textures;
		std::vector<std::shared_ptr<MaterialOpenGL>> materials;
		umap(std::string, uint32) materialNameToId;
		std::shared_ptr<MemoryOpenGL> memory;
		umap(uint64, uint32) instances;
		std::shared_ptr<Scene> scene;
		uint32 drawCalls = 0;

		bool init() override
		{
			info("OpenGL: %s %s (%s)", glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION));

			glEnable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

			memory = std::make_shared<MemoryOpenGL>();

			return true;
		};

		void setClearColor(vec4 color) override
		{
			glClearColor(color.r, color.g, color.b, color.a);
		};

		void setViewport(vec2 size, vec2 offset) override
		{
			glViewport((int)offset.x, (int)offset.y, (int)size.x, (int)size.y);
		};

		std::shared_ptr<Memory> getMemory() override
		{
			return memory;
		};

		std::shared_ptr<Shader> loadShader(const std::string& name) override
		{
			shaders[name] = std::make_shared<ShaderOpenGL>();
			shaders[name]->init(name, true);
			shaders[name]->compile();
			return shaders[name];
		};

		std::shared_ptr<Shader> getShader(const std::string& name) override
		{
			return shaders[name];
		};

		std::shared_ptr<Texture> loadTexture(const std::string& name) override
		{
			textures[name] = std::make_shared<Texture2DOpenGL>();
			textures[name]->init(name);
			return textures[name];
		};

		std::shared_ptr<Texture> getTexture(const std::string& name) override
		{
			return textures[name];
		};

		std::shared_ptr<Material> makeMaterial(const std::string& name, const std::string& shaderName, const std::vector<std::string>& textureNames) override
		{
			auto material = std::make_shared<MaterialOpenGL>();
			material->id = materials.size();
			materials.resize(material->id + 1);

			material->shader = shaders[shaderName];
			for (auto& tex : textureNames)
			{
				material->textures.push_back(textures[tex]);
			}

			materials[material->id] = material;
			materialNameToId[name] = material->id;
			return material;
		};

		std::shared_ptr<Material> getMaterial(const std::string& name) override
		{
			return materials[materialNameToId[name]];
		};

		std::shared_ptr<Material> getMaterial(uint32 id) override
		{
			return materials[id];
		};

		void onStart() override
		{
			scene = getParent<Modular>()->getModule<Scene>("scene");
		}

		void submit(InstanceMap& map) override
		{
			drawCalls = 0;

			uint64 instanceId = (uint64) map.material << 32 | map.mesh;

			auto buffer = memory->buffers[map.mesh];
			auto mat = materials[map.material];

			mat->shader->bind();
			mat->shader->uploadMat4("view", scene->getDefaultCamera()->view);
			mat->shader->uploadMat4("proj", scene->getDefaultCamera()->proj);
			mat->shader->unbind();

			if (!map.changed && instances.find(instanceId) != instances.end())
			{
				return;
			}

			instances[instanceId] = map.instances.size();
			map.changed = false;

			glBindBuffer(GL_ARRAY_BUFFER, buffer->instanceBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * map.instances.size(), &map.instances[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		};

		void draw() override
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			for (auto&& kv : instances)
			{
				auto buffer = memory->buffers[(uint32)kv.first];
				auto mat = materials[kv.first >> 32];

				mat->shader->bind();
				buffer->bind();

				// TODO: do during submit?
				if (!mat->textures.empty())
				{
					for (uint32 i = 0; i < mat->textures.size(); i++)
					{
						auto tex = mat->textures[i];
						tex->bind(i);
						mat->shader->uploadInt("uTexture" + std::to_string(i), i);
					}
				}

				if (buffer->indicesCount == 0)
				{
					glDrawArraysInstanced(GL_TRIANGLES, 0, buffer->verticesCount, kv.second);
				}
				else
				{
					glDrawElementsInstanced(GL_TRIANGLES, buffer->indicesCount, GL_UNSIGNED_INT, nullptr, kv.second);
				}

				buffer->unbind();
				mat->shader->unbind();

				drawCalls++;
			}
		};

		void onShutdown() override
		{
			memory->cleanup();
			for (auto&& kv : shaders)
			{
				kv.second->cleanup();
			}
			shaders.clear();
			for (auto&& kv : textures)
			{
				kv.second->cleanup();
			}
			textures.clear();
		};
	};
};
