#pragma once
#include <concurrentqueue.h>
#include <iostream>
#include "../Defines.hpp"

#define debug(x, ...) viper::log(viper::LogLevel::DEBUG, x, __VA_ARGS__);
#define info(x, ...) viper::log(viper::LogLevel::INFO, x, __VA_ARGS__);
#define warn(x, ...) viper::log(viper::LogLevel::WARNING, x, __VA_ARGS__);
#define crit(x, ...) viper::log(viper::LogLevel::CRITICAL, x, __VA_ARGS__);

namespace viper
{
	enum LogLevel
	{
		DEBUG = 0,
		INFO = 1,
		WARNING = 2,
		CRITICAL = 3
	};

	void queueLog(std::string message);
	void pollLogger(void(*func)(std::string));
	void pollLogger();

	template<typename ... Args>
	inline void log(LogLevel level, const std::string& message, const Args& ... args)
	{
		std::string prefix;
		switch (level)
		{
		case INFO:
			prefix = "[info] ";
			break;
		case WARNING:
			prefix = "[warn] ";
			break;
		case CRITICAL:
			prefix = "[critical] ";
			break;
		default:
			prefix = "[debug] ";
			break;
		}
		char buffer[1024];
		sprintf_s(buffer, (prefix + message).c_str(), args...);
		queueLog(std::string(buffer));
	};
};