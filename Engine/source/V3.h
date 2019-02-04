#pragma once
#include "Macros.h"
#include "config/ConfigLayer.h"
#include "view/ViewLayer.h"
#include "events/EventLayer.h"
#include "pipeline/Pipeline.h"
#include "EngineExtension.h"

#define v3i() V3::getInstance();

class V3
{
protected:
	V3();
public:
	static V3& getInstance()
	{
		static V3 instance;
		return instance;
	}

	V3API ~V3();

	std::shared_ptr<EventLayer> V3API getEvents();
	std::shared_ptr<ConfigLayer> V3API getConfig();
	std::shared_ptr<ViewLayer> V3API getView();
	std::shared_ptr<Pipeline> V3API getPipeline();

	std::string debugWindowTitle;
	unsigned int framesPerSecond = 0;
	double deltaTime = 0;
	std::atomic<double> elapsedTime = 0.0;

	typedef std::unordered_map<int, std::shared_ptr<Tickable>> TickMap;
	typedef std::unordered_map<int, std::shared_ptr<EngineExtension>> ExtensionMap;

	void V3API start();

	unsigned int V3API addTickable(std::shared_ptr<Tickable> object);
	unsigned int V3API addExtension(std::shared_ptr<EngineExtension> object, double interval = 0.0);
	void V3API removeTickable(unsigned int id);
	void V3API removeExtension(unsigned int id);
private:
	std::shared_ptr<EventLayer> events;
	std::shared_ptr<ConfigLayer> config;
	std::shared_ptr<ViewLayer> view;
	std::shared_ptr<Pipeline> pipeline;

	std::shared_ptr<TickMap> tickables;
	std::shared_ptr<ExtensionMap> extensions;
};
