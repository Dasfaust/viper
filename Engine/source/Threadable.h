#pragma once
#include "Macros.h"
#include "Tickable.h"
#include <atomic>
#include <functional>
#include <thread>
#include <tbb/concurrent_queue.h>

class Threadable : public Tickable
{
public:
	Threadable() { }
	virtual ~Threadable() { };

	virtual void poll()
	{
		while (running)
		{
			this->tick();
		}
	};

	virtual void start()
	{
		onStart();
		this->running = true;
		this->worker = std::thread(&Threadable::poll, this);
	};

	virtual void stop()
	{
		onStop();
		this->running = false;
		this->worker.join();
	};

	virtual void onStart() { };
	virtual void onStop() { };
private:
	std::atomic<bool> running = false;
	std::thread worker;
};