#pragma once
#include <memory>
#include "Module.hpp"
#include <map>

class Modular : public std::enable_shared_from_this<Modular>
{
public:
	virtual ~Modular() = default;

	int64 deltaTime = 0;
	float deltaTimeMs = 0.0f;
	float deltaTimeS = 0.0f;
	float tickTimeMs = 0.0f;
	Time::point lastTick;
	
	umap(std::string, std::shared_ptr<Module>) modules;

	virtual void tickModules()
	{
		auto start = Time::now();
		deltaTime = Time::since(start, lastTick);
		lastTick = start;
		deltaTimeMs = Time::toMilliseconds(deltaTime);
		deltaTimeS = Time::toSeconds(deltaTime);
		
		for (auto&& kv : modules)
		{
			kv.second->onTickBegin();
		}

		for (auto&& kv : modules)
		{
			if (kv.second->modInterval > 0.0f)
			{
				if (Time::toMilliseconds(kv.second->modAccumulator) >= kv.second->modInterval)
				{
					kv.second->modAccumulator = 0;
					kv.second->onTick();
				}
				else
				{
					kv.second->modAccumulator += deltaTime;
					kv.second->onTickWait();
				}
			}
			else
			{
				kv.second->onTick();
			}
		}

		for (auto&& kv : modules)
		{
			kv.second->onTickEnd();
		}

		tickTimeMs = Time::toMilliseconds(Time::since(start));
	};

	template<typename T>
	inline std::shared_ptr<T> initModule(const std::string& name, float interval = 0.0f)
	{
		auto mod = std::make_shared<T>();
		mod->parent = shared_from_this();
		mod->modInterval = interval;
		modules[name] = mod;
		return mod;
	};

	template<typename T>
	inline std::shared_ptr<T> getModule(const std::string& name)
	{
 		return std::dynamic_pointer_cast<T>(modules[name]);
	};

	inline std::shared_ptr<Module> getModule(const std::string& name)
	{
		return modules[name];
	};

	inline bool isModuleLoaded(const std::string& name) const
	{
		return modules.count(name) > 0;
	};
};