#include "V3.h"
#include <memory>
#include "Logger.h"
#include <boost/algorithm/string.hpp>

#ifdef V3_WIN64
#include <windows.h>
	std::string V3::getWorkingDirectory()
	{
		char result[MAX_PATH];
  		std::string res(result, GetModuleFileName(NULL, result, MAX_PATH));
		boost::replace_all(res, "Client\\", "");
		return res;
	}
#elif defined V3_LIN64
#include <unistd.h>
#include <limits.h>
	std::string V3::getWorkingDirectory()
	{
		char result[PATH_MAX];
		ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
		std::string res(result, (count > 0) ? count : 0);
		boost::replace_all(res, "Client/", "");
		return res;
	}
#endif

V3::V3(std::string workingDir)
{
	Log::start();
	events = std::make_shared<EventLayer>();
	config = std::make_shared<ConfigLayer>(workingDir, events);
	view = std::make_shared<ViewLayer>(events, config);

	addToRenderTicks(view);
}

V3::~V3()
{
	Log::stop();
}

std::shared_ptr<EventLayer> V3::getEvents()
{
	return events;
}

std::shared_ptr<ConfigLayer> V3::getConfig()
{
	return config;
}

std::shared_ptr<ViewLayer> V3::getView()
{
	return view;
}

void V3::start()
{
	while (!view->closeRequested())
	{
		for (auto tickable : renderTicks)
		{
			tickable->tick();
		}
	}

	info("Shutting down, goodbye.");
	renderTicks.empty();
	logicTicks.empty();
}

void V3::addToLogicTicks(std::shared_ptr<Tickable> object)
{
	logicTicks.push_back(object);
}

void V3::addToRenderTicks(std::shared_ptr<Tickable> object)
{
	renderTicks.push_back(object);
}
