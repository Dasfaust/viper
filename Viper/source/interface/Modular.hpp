#pragma once
#include <memory>
#include "Module.hpp"
#include <map>

class Modular : public std::enable_shared_from_this<Modular>
{
public:
	virtual ~Modular() = default;

	time_val dt = 0.0;
	time_val lastTickNs = 0.0;
	std::map<std::string, std::shared_ptr<Module>> modules;

	virtual void tickModules()
	{
		for (auto&& kv : modules)
		{
			kv.second->onTickBegin();
		}

		for (auto&& kv : modules)
		{
			if (kv.second->modInterval > 0)
			{
				if (kv.second->modAccumulator >= kv.second->modInterval)
				{
					kv.second->modAccumulator = 0.0;
					kv.second->onTick();
				}
				else
				{
					kv.second->modAccumulator += dt;
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

		dt = timesince(lastTickNs);
		lastTickNs = tnowns();
	};

	template<typename T>
	inline std::shared_ptr<T> initModule(const std::string& name, double interval = 0.0)
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