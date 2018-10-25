#include "Logger.h"
#include <thread>
#include <atomic>
#include <memory>
#include <tbb/concurrent_queue.h>

namespace Log
{
	static std::atomic<bool> running = false;
	static std::thread worker;
	static std::shared_ptr<tbb::concurrent_queue<std::shared_ptr<std::string>>> messages = std::make_shared<tbb::concurrent_queue<std::shared_ptr<std::string>>>();;
}

void Log::queue(Level level, std::string message)
{
	std::string input = "V3::" + message;
	std::shared_ptr<std::string> ptr = std::make_shared<std::string>(input);
	messages->emplace(ptr);
}

void Log::poll()
{
	while (running)
	{
		if (!messages->empty())
		{
			std::shared_ptr<std::string> result;
			if (messages->try_pop(result))
			{
				std::cout << (*result).c_str() << std::endl;
			}
		}
	}
}

void Log::start()
{
	running = true;
	worker = std::thread(poll);
}

void Log::stop()
{
	running = false;
	worker.join();
}
