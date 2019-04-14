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
	std::thread worker;
	std::atomic<bool> running = false;

	Threadable() { }
	virtual ~Threadable() { };

	virtual void poll()
	{
		onStart();
		while (running)
		{
			this->tick();
		}
		onStop();
	};

	virtual void start()
	{
		this->running = true;
		this->worker = std::thread(&Threadable::poll, this);
	};

	virtual void stop()
	{
		bool expected = true;
		while (!this->running.compare_exchange_weak(expected, false));
		this->worker.join();
	};

	virtual void onStart() { };
	virtual void onStop() { };
};