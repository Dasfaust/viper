#include "PipelineOpenGL.h"

PipelineOpenGL::PipelineOpenGL(std::shared_ptr<EventLayer> events, std::shared_ptr<ConfigLayer> config, std::shared_ptr<ViewLayer> view)
{
	this->events = events;
	this->config = config;
	this->view = view;
	info("Rendering API: OpenGL");
}

PipelineOpenGL::~PipelineOpenGL()
{

}

void PipelineOpenGL::tick()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(view->getWindow());
}