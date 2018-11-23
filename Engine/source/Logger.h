#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>

#define debug(x) Log::log(Log::DEBUG, x);
#define debugf(x, ...) Log::log(Log::DEBUG, x, __VA_ARGS__);
#define info(x) Log::log(Log::INFO, x);
#define infof(x, ...) Log::log(Log::INFO, x, __VA_ARGS__);
#define warn(x) Log::log(Log::WARNING, x);
#define warnf(x, ...) Log::log(Log::WARNING, x, __VA_ARGS__);
#define crit(x) Log::log(Log::CRITICAL, x);
#define critf(x, ...) Log::log(Log::CRITICAL, x, __VA_ARGS__);

namespace Log
{
	enum Level
	{
		DEBUG = 0,
		INFO = 1,
		WARNING = 2,
		CRITICAL = 3
	};

	void queue(Level level, std::string message);

	template<typename ... Args>
	inline void log(Level level, std::string message, Args ... args)
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
		std::string string = prefix + message;
		char buffer[256];
		std::sprintf(buffer, string.c_str(), args...);
		queue(level, buffer);
	}

	void poll();

	void start();

	void stop();
}