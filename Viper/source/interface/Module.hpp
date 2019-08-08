#pragma once
#include "../Defines.hpp"

class Modular;

class Module
{
public:
	virtual ~Module() = default;

	std::shared_ptr<Modular> parent;
	time_val modInterval;
	time_val modAccumulator;
	std::string friendlyName;

	template<typename T>
	std::shared_ptr<T> getParent()
	{
		return std::dynamic_pointer_cast<T>(parent);
	};

	virtual void onStart() { };
	virtual void onTickBegin() { };
	virtual void onTick() { };
	virtual void onTickEnd() { };
	virtual void onTickWait() { };
	virtual void onShutdown() { };
private:
};