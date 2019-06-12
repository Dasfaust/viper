#pragma once
#include "../interface/Module.hpp"
#include "../interface/Threadable.hpp"

class Threads : public Module
{
public:
	flatmap(uint8, std::shared_ptr<Threadable>) threads;

	template<typename T>
	std::shared_ptr<T> spawn()
	{
		uint8 id = (uint8)threads.size() + 1;
		threads[id] = std::make_shared<T>();
		threads[id]->id = id;
		threads[id]->start();
		return std::static_pointer_cast<T>(threads[id]);
	}

	void watch(std::shared_ptr<Threadable> thread)
	{
		uint8 id = (uint8)threads.size() + 1;
		threads[id] = thread;
		threads[id]->id = id;
	}

	void onShutdown() override
	{
		for(auto&& kv : threads)
		{
			kv.second->stop();
		}
	};
};