#pragma once
#include "../interface/Pipeline.hpp"
#include "glad/glad.h"
#include "ShaderOpenGL.hpp"
#include "MemoryOpenGL.hpp"
#include "TextureOpenGL.hpp"
#include "MaterialOpenGL.hpp"

namespace gfx
{
	struct Subscene
	{
		std::shared_ptr<MaterialOpenGL> material;
		std::shared_ptr<BufferViewOpenGL> buffer;
		InstanceMap* instances;
	};

	class PipelineOpenGL : public Pipeline
	{
	public:
		umap(std::string, std::shared_ptr<ShaderOpenGL>) shaders;
		umap(std::string, std::shared_ptr<Texture2DOpenGL>) textures;
		umap(std::string, std::shared_ptr<MaterialOpenGL>) materials;
		std::shared_ptr<MemoryOpenGL> memory;
		std::vector<Subscene> subscenes;
		uint32 drawCalls = 0;

		bool init() override
		{
			info("OpenGL: %s %s (%s)", glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION));

			glEnable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
			materials[name] = std::make_shared<MaterialOpenGL>();
			materials[name]->shader = shaders[shaderName];
			for (auto& tex : textureNames)
			{
				materials[name]->textures.push_back(textures[tex]);
			}
			return materials[name];
		};

		std::shared_ptr<Material> getMaterial(const std::string& name) override
		{
			return materials[name];
		};

		void submit(const std::string& materialName, const std::string& mesh, InstanceMap& instances) override
		{
			auto material = materials[materialName];
			auto buffer = memory->buffers[mesh];
			auto scene = getParent<Modular>()->getModule<Scene>("scene");
			Subscene sub = { material, buffer, &instances };
			material->shader->bind();
			material->shader->uploadMat4("view", scene->getDefaultCamera()->view);
			material->shader->uploadMat4("proj", scene->getDefaultCamera()->proj);
			material->shader->unbind();

			std::vector<mat4> models;
			for (auto&& kv : instances)
			{
				if (kv.second.is3D)
				{
					auto model = mat4(1.0f);
					model = glm::rotate(model, glm::radians(kv.second.transform3d.rotation), kv.second.transform3d.rotationAxis);
					model = glm::scale(model, kv.second.transform3d.scale);
					models.push_back(glm::translate(model, kv.second.transform3d.position));
				}
				else
				{
					mat4 model(1.0f);
					model = glm::translate(model, vec3(kv.second.transform2d.position, 0.0f));
					model = glm::scale(model, vec3(kv.second.transform2d.scale));
					models.push_back(model);
				}
			}
			glBindBuffer(GL_ARRAY_BUFFER, buffer->instanceBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * models.size(), &models[0], GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			subscenes.push_back(sub);
			drawCalls = 0;
		};

		void draw() override
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			for (auto& subscene : subscenes)
			{
				subscene.material->shader->bind();
				subscene.buffer->bind();

				if (!subscene.material->textures.empty())
				{
					for (uint32 i = 0; i < subscene.material->textures.size(); i++)
					{
						auto tex = subscene.material->textures[i];
						tex->bind(i);
						subscene.material->shader->uploadInt("uTexture" + std::to_string(i), i);
					}
				}

				if (subscene.buffer->indicesCount == 0)
				{
					glDrawArraysInstanced(GL_TRIANGLES, 0, subscene.buffer->verticesCount, subscene.instances->size());
				}
				else
				{
					glDrawElementsInstanced(GL_TRIANGLES, subscene.buffer->indicesCount, GL_UNSIGNED_INT, nullptr, subscene.instances->size());
				}

				drawCalls++;
				subscene.buffer->unbind();
				subscene.material->shader->unbind();
			}

			subscenes.clear();
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
