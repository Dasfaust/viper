#pragma once
#include "Pipeline.h"
#include "../V3.h"

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

	glm::mat4 projMatrix;

	V3API PipelineOpenGL(std::shared_ptr<ConfigLayer> config, std::shared_ptr<ViewLayer> view, std::shared_ptr<EventLayer> events);
	V3API ~PipelineOpenGL() override;

	void V3API tick() override;

	void createShader(std::string name) override;
	void meshToMemory(std::string name) override;
	void modelToMemory(std::string name) override;
	void meshToVRAM(std::string name) override;
	void modelToVRAM(std::string name) override;
	void textureToMemory(std::string name) override;
	void textureToVRAM(std::string name) override;
	unsigned int makeRenderCommand() override;
private:
};