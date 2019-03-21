#pragma once
#include "Macros.h"
#include "Logger.h"
#include "core/DeltaTime.h"
#include <unordered_map>
#include <atomic>

class V3
{
public:
	V3API V3();
	V3API ~V3();

	void V3API start();

	template<typename T>
	T* initModule(double interval = 0.0)
	{
		unsigned int id = moduleIndex++;
		debugf("Module init: %d (%s)", id, typeid(T).name());
		T* module = new T();
		module->v3 = this;
		module->interval = interval;
		modules[id] = module;
		return static_cast<T*>(modules[id]);
	};

	template<typename T>
	T* getModule()
	{
		for (auto kv : modules)
		{
			if (dynamic_cast<T*>((&kv)->second) != nullptr)
			{
				return static_cast<T*>((&kv)->second);
			}
		}
		critf("Module %s requested but is not initialized", typeid(T).name());
		return nullptr;
	};

	template<typename T>
	bool isModuleLoaded()
	{
		for (auto kv : modules)
		{
			if (dynamic_cast<T*>((&kv)->second) != nullptr)
			{
				return true;
			}
		}
		return false;
	}

	inline void shutdown()
	{
		bool expected = false;
		while(!running.compare_exchange_weak(expected, false));
	};
private:
	std::atomic<bool> running = true;

	unsigned int moduleIndex = 0;
	std::unordered_map<unsigned int, Module*> modules;

	DeltaTime* dt;
};

class V3Application : public Module
{
public:
	~V3Application() { };
};