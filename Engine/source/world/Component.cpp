#include "Component.h"

std::vector<std::tuple<ComponentBase::CreateComponentFunc, ComponentBase::RemoveComponentFunc, size_t>> ComponentBase::types;

unsigned int ComponentBase::registerType(CreateComponentFunc createfn, RemoveComponentFunc removefn, size_t size)
{
	unsigned int id = types.size();
	types.push_back(std::make_tuple(createfn, removefn, size));
	return id;
}