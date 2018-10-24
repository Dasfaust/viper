#include "V3.hpp"
#include "events/EventLayer.h"
#include <thread>

void doTask(V3 &v3)
{
	v3.getConfig()->setInts("engine", "testVal2", 75);
}

int main()
{
	info("Hello, world!");

	V3 v3("C:/Vulkan/V3/Client/resources");

	std::shared_ptr<EventListener<Event::OnConfigChangedData>> listener = std::make_shared<EventListener<Event::OnConfigChangedData>>(
		[](Event::OnConfigChangedData e) { debugf("OnConfigChanged: %s, %s", e.section.c_str(), e.segment.c_str()); }
	);
	v3.getEvents()->getOnConfigChanged()->addListener(listener);

	v3.getConfig()->setInts("engine", "testVal", 100);

	std::thread thread(doTask, std::ref(v3));
	
	while (true)
	{
		listener->poll();
	}

	return EXIT_SUCCESS;
}