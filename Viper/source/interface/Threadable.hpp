#pragma once
#include <atomic>
#include <thread>
#include "../Defines.hpp"
#include "../log/Logger.hpp"

class Threadable
{
public:
	std::thread worker;
	std::atomic_bool running = false;
	bool sleep = true;
	uint32 id = 999;

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
		if (id == 999)
		{
			info("Spawning unwatched thread");
		}
		else
		{
			info("Spawning thread: %d", id);
		}

		set_atom(running, true, bool);
		worker = std::thread(&Threadable::poll, this);
	};

	virtual void stop()
	{
		if (id == 999)
		{
			info("Stopping unwatched thread");
		}
		else
		{
			info("Stopping thread: %d", id);
		}

		set_atom(running, false, bool);
		worker.join();
	};

	virtual void onStartAsync() { };
	virtual void onStopAsync() { };
	virtual void onTickAsync() { };
};