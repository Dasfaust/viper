#pragma once
#include <atomic>
#include <thread>
#include "../Defines.hpp"

class Threadable
{
public:
	std::thread worker;
	std::atomic_bool running = false;
	bool sleep = true;
	uint8 id;

	Threadable() { }
	virtual ~Threadable() { };

	virtual void poll()
	{
		onStartAsync();
		while (running)
		{
			onTickAsync();

			if (sleep)
			{
				std::this_thread::sleep_for(std::chrono::nanoseconds(1000));
			}
		}
		onStopAsync();
	};

	virtual void start()
	{
		set_atom(running, true, bool);
		worker = std::thread(&Threadable::poll, this);
	};

	virtual void stop()
	{
		set_atom(running, false, bool);
		worker.join();
	};

	virtual void onStartAsync() { };
	virtual void onStopAsync() { };
	virtual void onTickAsync() { };
};