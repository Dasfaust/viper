#include "Logger.h"
#include <thread>
#include <atomic>
#include <memory>
#include "concurrentqueue.h"
#include <chrono>

namespace Log
{
	static std::atomic<bool> running = false;
	static std::thread worker;
	static std::shared_ptr<moodycamel::ConcurrentQueue<std::string>> messages = std::make_shared<moodycamel::ConcurrentQueue<std::string>>();;
}

void Log::queue(Level level, std::string message)
{
	std::string input = "V3::" + message;
	messages->enqueue(input);
}

void Log::poll()
{
	while (running)
	{
		if (checkQueue()){ }
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 30));
		}
	}
}

bool Log::checkQueue()
{
	std::string message;
	while(messages->try_dequeue(message))
	{
		std::cout << message.c_str() << std::endl;
	}
	return message.length();
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
