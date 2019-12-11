#pragma once
#include "interface/Modular.hpp"
#include <atomic>

class Viper : public Modular
{
public:
	static std::atomic_bool running;
	std::atomic_bool isInitialized = false;

	Viper();
	~Viper() = default;

	void onStart();
	void start();
	void onShutdown();

	static void shutdown()
	{
		set_atom(running, false, bool);
	}
private:
};