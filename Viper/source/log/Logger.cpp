#include "Logger.hpp"
#include "Logging.hpp"

bool Logging::async = true;

namespace viper
{
	static moodycamel::ConcurrentQueue<std::string> queue = moodycamel::ConcurrentQueue<std::string>();

	void pollLogger()
	{
		std::string msg;
		while (queue.try_dequeue(msg))
		{
			std::cout << msg.c_str() << std::endl;
		}
	};

	void pollLogger(void(*func)(std::string))
	{
		std::string msg;
		while (queue.try_dequeue(msg))
		{
			func(msg);
		}
	};

	void queueLog(std::string message)
	{
		if (Logging::async)
		{
			queue.enqueue(message);
		}
		else
		{
			std::cout << message.c_str() << std::endl;
		}
	};
};