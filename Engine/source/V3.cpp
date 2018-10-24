#include "V3.hpp"
#include <memory>

V3::V3(std::string workingDir)
{
	events = std::make_shared<EventLayer>();
	config = std::make_shared<ConfigLayer>(workingDir, events);
	view = std::make_shared<ViewLayer>(config.get());
}

V3::~V3()
{

}

EventLayer* V3::getEvents()
{
	return events.get();
}

ConfigLayer* V3::getConfig()
{
	return config.get();
}

ViewLayer* V3::getView()
{
	return view.get();
}
