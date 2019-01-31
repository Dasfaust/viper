#pragma once
#include "Pipeline.h"

class PipelineOpenGL : public Pipeline
{
public:
	V3API PipelineOpenGL(std::shared_ptr<EventLayer> events, std::shared_ptr<ConfigLayer> config, std::shared_ptr<ViewLayer> view);
	V3API ~PipelineOpenGL() override;

	void V3API tick() override;

private:
	std::shared_ptr<ConfigLayer> config;
	std::shared_ptr<EventLayer> events;
	std::shared_ptr<ViewLayer> view;
};