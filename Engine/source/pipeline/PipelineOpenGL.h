#pragma once
#include "Pipeline.h"
#include "../view/ViewLayer.h"
#include "../config/ConfigLayer.h"

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

	class OGLModel : public Pipeline::Model { };

	class OGLTexture : public Pipeline::Texture { };

	boost::container::flat_map<std::string, std::shared_ptr<OGLShader>> loadedShaders;
	boost::container::flat_map<std::string, std::shared_ptr<OGLMesh>> loadedMeshes;
	boost::container::flat_map<std::string, std::shared_ptr<OGLModel>> loadedModels;
	boost::container::flat_map<std::string, std::shared_ptr<OGLTexture>> loadedTextures;

	V3API PipelineOpenGL();
	V3API void onStartup() override;
	V3API ~PipelineOpenGL() override;

	void V3API onTick() override;

	void V3API loadResources(MeshComponent mc);
	void V3API createShader(std::string name) override;
	void V3API meshToMemory(std::string name) override;
	void V3API modelToMemory(std::string name) override;
	void V3API meshToVRAM(std::string name) override;
	void V3API modelToVRAM(std::string name) override;
	void V3API textureToMemory(std::string name) override;
	void V3API textureToVRAM(std::string name) override;

	ConfigLayer* config;
	ViewLayer* view;
};