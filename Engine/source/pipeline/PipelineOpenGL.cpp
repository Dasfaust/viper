#include "PipelineOpenGL.h"
#include "glm/vec3.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include "../util/FileUtils.h"
//#include "imgui_impl_opengl3.h"

#define ASSERT(x) if (!(x)) throw std::runtime_error("Fatal error in rendering loop");
#define _(x) poglClearError(); x; ASSERT(poglCheckError(#x, __FILE__, __LINE__));

static void poglClearError()
{
	while(glGetError() != GL_NO_ERROR);
}

static bool poglCheckError(const char* function, const char* file, int line)
{
	while(GLenum error = glGetError())
	{
		critf("OpenGL: exception %d at %s:%s:%d", error, file, function, line);
		return false;
	}

	return true;
}

// TODO abstract away
static void charCallback(GLFWwindow* window, unsigned int c)
{
	/*ImGuiIO& io = ImGui::GetIO();
	if (c > 0 && c < 0x10000)
	{
		io.AddInputCharacter((unsigned short)c);
	}*/
}

void PipelineOpenGL::OGLShader::setUniform(std::string name, Pipeline::Shader::UniformValue value)
{
	auto getLocation = [](std::string n, OGLShader shader) -> int
	{
		if (shader.uniformLocations.find(n) != shader.uniformLocations.end())
		{
			int loc = shader.uniformLocations[n];
			if (loc == -1)
			{
				warnf("Uniform variable '%s' does not have a location.", n.c_str());
			}
			return loc;
		}
		else
		{
			_(int location = glGetUniformLocation(shader.id, n.c_str()));
			shader.uniformLocations[n] = location;
			return location;
		}
	};
	
	switch(value.which())
	{
		case UNIFORM_INT:
		{
			auto val = boost::get<int>(value);
			int loc = getLocation(name, (*this));
			_(glUniform1i(loc, val));
			break;
		}
		case UNIFORM_FLOAT:
		{
			auto val = boost::get<float>(value);
			int loc = getLocation(name, (*this));
			_(glUniform1f(loc, val));
			break;
		}
		case UNIFORM_VEC2:
		{
			break;
		}
		case UNIFORM_VEC3:
		{
			break;
		}
		case UNIFORM_VEC4:
		{
			auto val = boost::get<glm::vec4>(value);
			int loc = getLocation(name, (*this));
			_(glUniform4f(loc, val.x, val.y, val.z, val.w));
			break;
		}
		case UNIFORM_MAT4:
		{
			auto val = boost::get<glm::mat4>(value);
			int loc = getLocation(name, (*this));
			_(glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(val)));
			break;
		}
	}
}

class BasicRenderer : public RenderCommand
{
public:
	PipelineOpenGL* pipeline;

	BasicRenderer(PipelineOpenGL* pipeline)
	{
		this->pipeline = pipeline;
	}

	void onObjectAdd(unsigned int id) override
	{
		auto object = objects[id];

		if (!object->shader.empty())
		{
			if (pipeline->loadedShaders.find(object->shader) == pipeline->loadedShaders.end())
			{
				pipeline->createShader(object->shader);
			}
		}

		if (!object->mesh.empty())
		{
			if (pipeline->loadedMeshes.find(object->mesh) == pipeline->loadedMeshes.end())
			{
				pipeline->meshToMemory(object->mesh);
				pipeline->meshToVRAM(object->mesh);
			}
		}

		if (!object->model.empty())
		{
			if (pipeline->loadedModels.find(object->model) == pipeline->loadedModels.end())
			{
				pipeline->modelToMemory(object->model);
				pipeline->modelToVRAM(object->model);
			}
		}

		if (!object->texture.empty())
		{
			if (pipeline->loadedTextures.find(object->texture) == pipeline->loadedTextures.end())
			{
				pipeline->textureToMemory(object->texture);
				pipeline->textureToVRAM(object->texture);
			}
		}
	};

	void rotateCamera()
	{
		/*float xOffset = V3::getInstance()->getPipeline()->camera->mouseViewport.x - V3::getInstance()->getPipeline()->camera->_mouseViewport.x;
		float yOffset = V3::getInstance()->getPipeline()->camera->_mouseViewport.y - V3::getInstance()->getPipeline()->camera->mouseViewport.y; // y starts at bottom
		V3::getInstance()->getPipeline()->camera->_mouseViewport.x = V3::getInstance()->getPipeline()->camera->mouseViewport.x;
		V3::getInstance()->getPipeline()->camera->_mouseViewport.y = V3::getInstance()->getPipeline()->camera->mouseViewport.y;

		float sensitivity = 0.15f;
		xOffset *= sensitivity;
		yOffset *= sensitivity;*/

		auto camera = V3::getInstance()->getPipeline()->camera;
		if (camera->mouseViewport != camera->_mouseViewport)
		{
			float xOffset = V3::getInstance()->getPipeline()->camera->mouseViewport.x * 0.001f;
			float yOffset = V3::getInstance()->getPipeline()->camera->mouseViewport.y * 0.001f;

			camera->_mouseViewport = camera->mouseViewport;

			V3::getInstance()->getPipeline()->camera->pitch += yOffset;
			V3::getInstance()->getPipeline()->camera->yaw += xOffset;
		}
	}

	void tick() override
	{
		if ((*pipeline->view->buttonStates)[2].pressed)
		{
			rotateCamera();
		}

		if (pipeline->camera->position.y != pipeline->camera->distance)
		{
			float lerp = pipeline->camera->distance * ((float)pipeline->deltaTime / 1000.0f * pipeline->camera->zoomSpeed) + pipeline->camera->position.y * ((float)(1.0f - pipeline->deltaTime / 1000.0f * pipeline->camera->zoomSpeed));
			pipeline->camera->position.y = lerp;
		}

		if (pipeline->camera->floorPosition != pipeline->camera->floorPositionTarget)
		{
			glm::vec3 lerp = pipeline->camera->floorPositionTarget * ((float)pipeline->deltaTime / 1000.0f * pipeline->camera->moveSpeed) + pipeline->camera->floorPosition * ((float)(1.0f - pipeline->deltaTime / 1000.0f * pipeline->camera->moveSpeed));
			pipeline->camera->floorPosition = lerp;
			pipeline->camera->position.x = lerp.x;
			pipeline->camera->position.z = lerp.z;
		}


		pipeline->camera->projection = glm::perspective(glm::radians(pipeline->camera->fov), pipeline->view->viewWidth / pipeline->view->viewHeight, 0.1f, 1000.0f);
		pipeline->camera->update();

		//debugf("a: %0.2f", pipeline->alpha.load());

		glm::vec3 coords;
		for(auto&& kv : objects)
		{
			auto object = kv.second;

			auto shader = pipeline->loadedShaders[object->shader];
			_(glUseProgram(shader->id));

			if (!object->texture.empty())
			{
				auto texture = pipeline->loadedTextures[object->texture];
				int textureSlot = object->textureSlot;
				_(glActiveTexture(GL_TEXTURE0 + textureSlot));
				_(glBindTexture(GL_TEXTURE_2D, texture->id));
				shader->setUniform("uTexture", textureSlot);
			}

			if (!object->mesh.empty())
			{
				// TODO proper instancing
				
				for (auto state : object->instances)
				{
					auto mesh = pipeline->loadedMeshes[object->mesh];
					glm::mat4 model = glm::mat4(1.0f);
					
					coords = state.worldCoordinates * ((float)pipeline->alpha) + state._worldCoordinates * ((float)(1.0f - pipeline->alpha));
					model = glm::translate(model, coords);

					float rotX = state.rotationX * ((float)pipeline->alpha) + state._rotationX * ((float)(1.0f - pipeline->alpha));
					float rotY = state.rotationY * ((float)pipeline->alpha) + state._rotationY * ((float)(1.0f - pipeline->alpha));
					float rotZ = state.rotationZ * ((float)pipeline->alpha) + state._rotationZ * ((float)(1.0f - pipeline->alpha));
					model = glm::rotate(model, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
					model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
					model = glm::rotate(model, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));

					glm::vec3 scale = state.scale * ((float)pipeline->alpha) + state._scale * ((float)(1.0f - pipeline->alpha));
					model = glm::scale(model, scale);

					shader->setUniform("model", model);
					shader->setUniform("view", pipeline->camera->view);
					shader->setUniform("projection", pipeline->camera->projection);

					_(glBindVertexArray(mesh->vaBuffer));
					if (mesh->indices.size() >= 3)
					{
						_(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer));
						_(glDrawElements(GL_TRIANGLES, (GLsizei)mesh->indices.size(), GL_UNSIGNED_INT, nullptr));
					}
					else
					{
						_(glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer));
						_(glDrawArrays(GL_TRIANGLES, 0, 36));
					}
				}
			}
		}
	};
};

unsigned int PipelineOpenGL::makeRenderCommand()
{
	auto command = std::make_shared<BasicRenderer>(this);

	int id = addRenderCommand(command);

	return id;
}

PipelineOpenGL::PipelineOpenGL(std::shared_ptr<ConfigLayer> config, std::shared_ptr<ViewLayer> view, std::shared_ptr<EventLayer> events)
{
	this->config = config;
	this->view = view;
	this->events = events;
	this->renderCommands = std::make_shared<tbb::concurrent_unordered_map<unsigned int, std::shared_ptr<RenderCommand>>>();
	this->loadedShaders = tbb::concurrent_unordered_map<std::string, std::shared_ptr<OGLShader>>();
	this->loadedMeshes = tbb::concurrent_unordered_map<std::string, std::shared_ptr<OGLMesh>>();
	this->loadedModels = tbb::concurrent_unordered_map<std::string, std::shared_ptr<OGLModel>>();
	this->loadedTextures = tbb::concurrent_unordered_map<std::string, std::shared_ptr<OGLTexture>>();

	camera = std::make_shared<Camera>();

	info("Rendering API: OpenGL");

	config->load("meshes", ConfigLayer::TYPE_CONFIG);
	config->load("shaders", ConfigLayer::TYPE_CONFIG);
	config->load("models", ConfigLayer::TYPE_CONFIG);
	config->load("textures", ConfigLayer::TYPE_CONFIG);

	_(glEnable(GL_DEPTH_TEST));

	camera->viewportSize = glm::vec2(view->viewWidth, view->viewHeight);

	camera->mouseListener = std::make_shared<EventListener<ViewEvents::OnMouseEventData>>(
		[](ViewEvents::OnMouseEventData e)
		{
			if (!e.cancelled)
			{
				V3::getInstance()->getPipeline()->camera->mouseViewport = e.coordinates;

				//ImGuiIO& io = ImGui::GetIO();
				//io.MouseWheel += e.scroll.y;
				//io.MouseWheelH += e.scroll.x;
				//io.MousePos = ImVec2(e.coordinates.x, e.coordinates.y);

				float zoom = e.scroll.y * V3::getInstance()->getPipeline()->camera->zoomSensitivity;
				V3::getInstance()->getPipeline()->camera->distance = glm::clamp(V3::getInstance()->getPipeline()->camera->distance - zoom, V3::getInstance()->getPipeline()->camera->minZoom, V3::getInstance()->getPipeline()->camera->maxZoom);

				for (auto&& kv : e.buttons)
				{
					//io.MouseDown[kv.first] = kv.second.pressed;
					//debugf("Mouse button state: %d: %d, %d", kv.first, kv.second.pressed, kv.second.released);
				}

				// event data doesn't work here. idk
				if ((*V3::getInstance()->getView()->buttonStates)[2].pressed)
				{
					glfwSetInputMode(V3::getInstance()->getView()->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				}
				else
				{
					glfwSetInputMode(V3::getInstance()->getView()->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				}
			}
		}
	);
	view->mouseEvent->addListener(camera->mouseListener);

	camera->keyListener = std::make_shared<EventListener<ViewEvents::OnKeyEventData>>(
		[](ViewEvents::OnKeyEventData e)
		{
			for (auto&& kv : e.buttons)
			{
				//debugf("Key state: %d: %d, %d", kv.first, kv.second.pressed, kv.second.released);

				/*ImGuiIO& io = ImGui::GetIO();
				if (kv.second.pressed)
				{
					io.KeysDown[kv.first] = true;
				}
				else
				{
					io.KeysDown[kv.first] = false;
				}*/
			}
		}
	);
	view->keyEvent->addListener(camera->keyListener);

	glfwSetCharCallback(view->getWindow(), charCallback);
}

PipelineOpenGL::~PipelineOpenGL()
{
	
}

void PipelineOpenGL::tick()
{
	if (glfwGetKey(view->getWindow(), GLFW_KEY_W) == GLFW_PRESS)
	{
		camera->moveForward(deltaTime);
		//camera->moveEvent->triggerEvent(camera);
	}
	if (glfwGetKey(view->getWindow(), GLFW_KEY_S) == GLFW_PRESS)
	{
		camera->moveBack(deltaTime);
		//camera->moveEvent->triggerEvent(camera);
	}
	if (glfwGetKey(view->getWindow(), GLFW_KEY_A) == GLFW_PRESS)
	{
		camera->moveLeft(deltaTime);
		//camera->moveEvent->triggerEvent(camera);
	}
	if (glfwGetKey(view->getWindow(), GLFW_KEY_D) == GLFW_PRESS)
	{
		camera->moveRight(deltaTime);
		//camera->moveEvent->triggerEvent(camera);
	}

	camera->update();
	camera->pollEvents();

	for(auto command : (*renderCommands))
	{
		command.second->onPreRender();
	}

	_(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	for(auto command : (*renderCommands))
	{
		command.second->tick();
	}

	for(auto command : (*renderCommands))
	{
		command.second->onPostRender();
	}

	glfwSwapBuffers(view->getWindow());
}

static unsigned int compileShader(unsigned int type, const std::string& source)
{
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	_(glShaderSource(id, 1, &src, nullptr));
	_(glCompileShader(id));

	int status;
	_(glGetShaderiv(id, GL_COMPILE_STATUS, &status));
	if (status == GL_FALSE)
	{
		int length;
		_(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		char* message = (char*)alloca(length * sizeof(char));
		_(glGetShaderInfoLog(id, length, &length, message));
		critf("Failed to compile shader: %d", type);
		crit(std::string(message));
		_(glDeleteShader(id));
		return 0;
	}

	return id;
}

void PipelineOpenGL::createShader(std::string name)
{
	debugf("Loading shader: %s", name.c_str());
	auto vertex = config->getStrings("shaders", name + "_vertex")[0];
	auto fragment = config->getStrings("shaders", name + "_fragment")[0];

	unsigned int program = glCreateProgram();
	unsigned int vs = compileShader(GL_VERTEX_SHADER, vertex);
	unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragment);

	_(glAttachShader(program, vs));
	_(glAttachShader(program, fs));
	_(glLinkProgram(program));
	_(glValidateProgram(program));

	_(glDeleteShader(vs));
	_(glDeleteShader(fs));

	auto shader = std::make_shared<OGLShader>();
	shader->id = program;
	shader->name = name;

	loadedShaders[name] = shader;
};

void PipelineOpenGL::meshToMemory(std::string name)
{
	debugf("Loading mesh: %s", name.c_str());
	auto verts = config->getFloats("meshes", name);
	auto mesh = std::make_shared<OGLMesh>();

	for (int f = 0; f < verts.size(); f++)
	{
		mesh->vertices.push_back(verts[f]);
	}

	auto attrib = config->getInts("meshes", name + "_attrib");
	for (int f = 0; f < attrib.size(); f++)
	{
		mesh->attribLayout.push_back(attrib[f]);
	}

	auto inds = config->getInts("meshes", name + "_indices");
	for (int f = 0; f < inds.size(); f++)
	{
		mesh->indices.push_back(inds[f]);
	}
	mesh->name = name;

	loadedMeshes[name] = mesh;
}

void PipelineOpenGL::modelToMemory(std::string name)
{
	debugf("Loading model: %s", name.c_str());
	std::shared_ptr<OGLModel> model = std::make_shared<OGLModel>();
	model->name = name;

	auto meshes = config->getStrings("models", name);
	for (std::string mesh : meshes)
	{
		if (loadedMeshes.find(mesh) == loadedMeshes.end())
		{
			meshToMemory(mesh);
		}
		model->meshes.push_back(mesh);
	}
	loadedModels[name] = model;
}

void PipelineOpenGL::textureToMemory(std::string name)
{
	debugf("Texture to memory: %s", name.c_str());
	auto texture = std::make_shared<OGLTexture>();
	texture->filePath = FileUtils::getWorkingDirectory() + FileUtils::getPathSeperator() + "resources" + FileUtils::getPathSeperator() + "textures" + FileUtils::getPathSeperator() + config->getStrings("textures", name)[0];
	debug(texture->filePath.c_str());
	stbi_set_flip_vertically_on_load(1);
	texture->buffer = stbi_load(texture->filePath.c_str(), &texture->width, &texture->height, &texture->bpp, 4);
	loadedTextures[name] = texture;
}

void PipelineOpenGL::meshToVRAM(std::string name)
{
	debugf("Mesh to GPU: %s", name.c_str());
	auto mesh = loadedMeshes[name];

	// Must be before vertex buffer
	unsigned int vaBuffer;
	_(glGenVertexArrays(1, &vaBuffer));
	_(glBindVertexArray(vaBuffer));
	mesh->vaBuffer = vaBuffer;
	
	unsigned int vertexBuff;
	_(glGenBuffers(1, &vertexBuff));
	_(glBindBuffer(GL_ARRAY_BUFFER, vertexBuff));
	_(glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(float), &mesh->vertices[0], GL_STATIC_DRAW));
	mesh->vertexBuffer = vertexBuff;
	mesh->vertices.clear();

	// Binds to currently bound buffer
	for (int p = 0; p < mesh->attribLayout.size(); p += 4)
	{
		_(glEnableVertexAttribArray(mesh->attribLayout[p]));
		_(glVertexAttribPointer(mesh->attribLayout[p], mesh->attribLayout[p + 1], GL_FLOAT, GL_FALSE, sizeof(float) * mesh->attribLayout[p + 2], (void*) (mesh->attribLayout[p + 3] * sizeof(float))));
		debugf("Attrib layout id: %d index: %d length: %d stride: %d offset: %d", p, mesh->attribLayout[p], mesh->attribLayout[p + 1], sizeof(float) * mesh->attribLayout[p + 2], (mesh->attribLayout[p + 3] * sizeof(float)));
	}

	unsigned int indexBuffer;
	_(glGenBuffers(1, &indexBuffer));
	_(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer));
	_(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(unsigned int), &mesh->indices[0], GL_STATIC_DRAW));
	mesh->indexBuffer = indexBuffer;
	//mesh->indices.clear();
}

void PipelineOpenGL::modelToVRAM(std::string name)
{
	debugf("Model to GPU: %s", name.c_str());
	for (std::string mesh : loadedModels[name]->meshes)
	{
		meshToVRAM(mesh);
	}
}

void PipelineOpenGL::textureToVRAM(std::string name)
{
	debugf("Texture to GPU: %s", name.c_str());
	auto texture = loadedTextures[name];

	_(glGenTextures(1, &texture->id));
	_(glBindTexture(GL_TEXTURE_2D, texture->id));

	_(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	_(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	_(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	_(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	_(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->buffer));

	_(glBindTexture(GL_TEXTURE_2D, 0));

	if (texture->buffer)
	{
		stbi_image_free(texture->buffer);
	}
}