#pragma once
#include "../Defines.hpp"

class Modular;

class Module
{
public:
	virtual ~Module() = default;

	Modular* parent;
	time_val modInterval;
	time_val modAccumulator;

	template<typename T>
	inline T* getParent()
	{
		return reinterpret_cast<T*>(parent);
	};

	virtual void onStart() { };
	virtual void onTickBegin() { };
	virtual void onTick() { };
	virtual void onTickEnd() { };
	virtual void onTickWait() { };
	virtual void onShutdown() { };
private:
};