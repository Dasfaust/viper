#include "Viper.hpp"
#include "event/Events.hpp"
#include "log/Logger.hpp"
#include <boost/exception/all.hpp>
#include "log/Logging.hpp"
#include "Server.hpp"

struct TestEvent : Event
{
	float data;
};

struct TestPacket : Packet
{
	float someFloat;
	bool someBool;

	template<class A>
	void serialize(A& ar)
	{
		ar(CEREAL_NVP(someFloat), CEREAL_NVP(someBool));
	};
};

class TestThread : public Threadable
{
public:
	Viper* viper;
	std::shared_ptr<Listener<TestEvent>> listener;

	TestThread(Viper* viper)
	{
		this->viper = viper;
		auto events = viper->getModule<Events>("events");
		auto handler = events->getModule<EventHandler<TestEvent>>("testevent");
		listener = handler->listen(1, [](TestEvent& ev, Viper* viper)
		{
			debug("Event fired on position 1, data: %.2f", ev.data);
		});
	};

	void onStartAsync() override
	{
		
	};

	void onTickAsync() override
	{
		listener->poll();
	};
};

class Sandbox : public Module
{
public:
	std::shared_ptr<EventHandler<TestEvent>> handler;
	std::shared_ptr<Listener<TestEvent>> listener;
	std::shared_ptr<PacketHandler<TestPacket>> packetHandler;
	std::shared_ptr<Listener<TestPacket>> packetListener;

	void onStart() override
	{
		auto events = getParent<Viper>()->getModule<Events>("events");
		handler = events->initModule<EventHandler<TestEvent>>("testevent");
		listener = handler->listen(0, [](TestEvent& ev, Viper* viper)
		{
			debug("Event fired on position 0, data: %.2f", ev.data);
			ev.data = 420.0;
		});

		auto thread = std::make_shared<TestThread>(getParent<Viper>());
		getParent<Viper>()->getModule<Threads>("threads")->watch(thread);
		thread->start();

		handler->fire({ false, 3.14f });

		auto net = getParent<Viper>()->getModule<Server>("server")->getModule<Networking>("networking");
		packetHandler = net->registerPacket<TestPacket>(100);
		packetListener = packetHandler->listen(0, [](TestPacket& packet, Viper* viper)
		{
			debug("Packet event: TestPacket: %.2f, %d", packet.someFloat, packet.someBool);
		}, getParent<Viper>());

		TestPacket p;
		p.someBool = false;
		p.someFloat = 69.69f;
		packetHandler->enqueue(p);
	};

	void onTick() override
	{
		listener->poll();
		packetListener->poll();
	};
};

int main()
{
	try
	{
		auto vi = std::make_shared<Viper>();
		vi->initModule<Threads>("threads");
		vi->initModule<Logging>("logging");
		vi->initModule<Events>("events");
		vi->initModule<Server>("server");
		vi->initModule<Sandbox>("game");
		vi->start();
	}
	catch(...)
	{
		std::clog << boost::current_exception_diagnostic_information() << std::endl;
	}
};