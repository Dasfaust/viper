#pragma once
#include "Entity.hpp"
#include "../interface/Module.hpp"
#include "../interface/Threadable.hpp"

namespace ecs
{
	class System
	{
	public:
		uint32 componentId;
		void(*updateEntity)(Entity* entity, void* component, std::shared_ptr<System> self, float dt);

		virtual void onTickBegin() { };
	};

	struct Job
	{
		uint64 start = 0;
		uint64 end = 0;
		bool valid = false;
		uint64 iterated = 0;
	};

	class Worker : public Module, public Threadable
	{
	public:
		uint32 id;
		moodycamel::ConcurrentQueue<Job> jobs;
		moodycamel::ConcurrentQueue<Job> completed;
		Job currentJob = { };

		void onTickAsync() override;

		void onTick() override
		{
			onTickAsync();
		}
	};
};

