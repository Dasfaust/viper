#pragma once
#include "interface/Module.hpp"

class World : public Module, public Modular
{
public:
	
	void onTick() override
	{
		tickModules();
	};
};