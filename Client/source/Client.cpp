#include "V3.hpp"
#include "events/EventLayer.h"

class Derp : public EventListener<Event::OnConfigChangedData>
{
public:
	Derp() { }

	void callback(std::shared_ptr<Event::OnConfigChangedData> data) override
	{
		debugf("Config changed: %s, %s", data->section.c_str(), data->segment.c_str());
	}
};

int main()
{
	info("Hello, world!");

	V3 v3("C:/Vulkan/V3/Client/resources");

	std::shared_ptr<EventListener<Event::OnConfigChangedData>> derp = std::make_shared<Derp>();
	v3.getEvents()->getOnConfigChanged()->addListener(derp);

	std::shared_ptr<Event::OnConfigChangedData> data = std::make_shared<Event::OnConfigChangedData>();
	data->section = "derp";
	data->segment = "derp";
	v3.getEvents()->getOnConfigChanged()->triggerEvent(data);

	v3.getConfig()->setInts("engine", "testVal", 100);

	return EXIT_SUCCESS;
}