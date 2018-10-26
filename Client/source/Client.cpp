#include "V3.hpp"
#include "events/EventLayer.h"
#include <thread>
#include "Logger.h"

void doTask(V3 &v3)
{
	v3.getConfig()->setInts("engine", "testVal2", 75);
}

class Test : public Tickable
{
public:
	std::shared_ptr<EventListener<Event::OnConfigChangedData>> listener;

	Test(V3 &v3)
	{
		listener = std::make_shared<EventListener<Event::OnConfigChangedData>>(
			[](Event::OnConfigChangedData e) { debug("OnConfigChanged: %s, %s", e.section.c_str(), e.segment.c_str()); }
		);
		v3.getEvents()->getOnConfigChanged()->addListener(listener);
	}

	void tick() override
	{
		listener->poll();
	}
};

int main()
{
	info("Hello, world!");

	V3 v3("C:/Vulkan/V3/Client/resources");

	v3.getView()->setTitle("TestGame");

	std::shared_ptr<Test> test = std::make_shared<Test>(v3);
	v3.addToRenderTicks(test);

	v3.getConfig()->setInts("engine", "testVal", 100);

	std::thread thread(doTask, std::ref(v3));
	
	v3.start();

	thread.join();

	return EXIT_SUCCESS;
}