#pragma once
#include "../interface/Module.hpp"
#include "../interface/Threadable.hpp"
#include "../Viper.hpp"
#include "../thread/Threads.hpp"

class Logging : public Module, public Threadable
{
public:
	static bool async;

	void onStart() override
	{
		if (async)
		{
			getParent<Viper>()->getModule<Threads>("threads")->watch(getParent<Viper>()->getModule<Logging>("logging"));
			start();
		}
	};

	void onTickAsync() override
	{
		viper::pollLogger();
	};
};
