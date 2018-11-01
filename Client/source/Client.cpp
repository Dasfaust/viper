#include "V3.h"
#include "events/EventLayer.h"
#include "Threadable.h"
#include "Logger.h"

void doTask(V3 &v3)
{
	v3.getConfig()->setInts("engine", "testVal2", 75);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	v3.getConfig()->setInts("engine", "otherVal4", 64);
}

class Thread : public Threadable
{
public:
	std::shared_ptr<EventListener<Event::OnConfigChangedData>> listener;

	Thread(V3 &v3)
	{
		listener = std::make_shared<EventListener<Event::OnConfigChangedData>>(
			[](Event::OnConfigChangedData e) { debug("OnConfigChanged (thread 2): %s, %s, %d", e.section.c_str(), e.segment.c_str(), boost::get<int>(e.values[0])); }
		);
		v3.getEvents()->getOnConfigChanged()->addListener(listener);
	}

	void tick() override
	{
		listener->poll();
	}
};

class Test : public Tickable
{
public:
	std::shared_ptr<EventListener<Event::OnConfigChangedData>> listener;

	Test(V3 &v3)
	{
		listener = std::make_shared<EventListener<Event::OnConfigChangedData>>(
			[](Event::OnConfigChangedData e) { debug("OnConfigChanged: %s, %s, %d", e.section.c_str(), e.segment.c_str(), boost::get<int>(e.values[0])); }
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

	Thread t(v3);
	t.start();

	v3.getConfig()->setInts("engine", "testVal", 100);

	std::thread thread(doTask, std::ref(v3));
	
	v3.start();

	t.stop();
	thread.join();

	return EXIT_SUCCESS;
}