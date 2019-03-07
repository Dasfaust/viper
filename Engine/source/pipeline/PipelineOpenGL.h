#pragma once
#include "Pipeline.h"
#include "../V3.h"
#include "../world/World.h"

class PipelineOpenGL : public Pipeline
{
public:
	class OGLMesh : public Pipeline::Mesh
	{
	public:
		unsigned int vertexBuffer;
		unsigned int indexBuffer;
		unsigned int vaBuffer;
	};

	class OGLShader : public Pipeline::Shader
	{
	public:
		std::unordered_map<std::string, int> uniformLocations;

		void setUniform(std::string name, UniformValue value) override;
	};

	class OGLModel : public Pipeline::Model
	{

	};

	class OGLTexture : public Pipeline::Texture
	{

	};

	tbb::concurrent_unordered_map<std::string, std::shared_ptr<OGLShader>> loadedShaders;
	tbb::concurrent_unordered_map<std::string, std::shared_ptr<OGLMesh>> loadedMeshes;
	tbb::concurrent_unordered_map<std::string, std::shared_ptr<OGLModel>> loadedModels;
	tbb::concurrent_unordered_map<std::string, std::shared_ptr<OGLTexture>> loadedTextures;

	std::shared_ptr<ConfigLayer> config;
	std::shared_ptr<ViewLayer> view;
	std::shared_ptr<EventLayer> events;

	V3API PipelineOpenGL(std::shared_ptr<ConfigLayer> config, std::shared_ptr<ViewLayer> view, std::shared_ptr<EventLayer> events);
	V3API ~PipelineOpenGL() override;

	void V3API tick() override;

	void V3API createShader(std::string name) override;
	void V3API meshToMemory(std::string name) override;
	void V3API modelToMemory(std::string name) override;
	void V3API meshToVRAM(std::string name) override;
	void V3API modelToVRAM(std::string name) override;
	void V3API textureToMemory(std::string name) override;
	void V3API textureToVRAM(std::string name) override;
	unsigned int V3API makeRenderCommand() override;

	class RenderComponent : public Component<RenderComponent>
	{
	public:
		glm::vec3 locationPrevious;
		glm::vec3 locationCurrent;
	};

	tbb::concurrent_vector<RenderComponent> components;
private:
};