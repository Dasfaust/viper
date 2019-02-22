#pragma once
#include "Component.h"

class SystemBase
{
public:
	SystemBase(const std::vector<unsigned int> types) : types(types) { };

	virtual void update(double deltaTime, ComponentBase** components);

	const std::vector<unsigned int> getTypes()
	{
		return types;
	};
private:
	std::vector<unsigned int> types;
};