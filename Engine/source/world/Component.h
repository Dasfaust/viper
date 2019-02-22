#pragma once
#include <vector>
#include <tuple>
#include "../Macros.h"

typedef void* EntityHandle;

struct ComponentBase
{
public:
	typedef unsigned int(*CreateComponentFunc)(std::vector<unsigned int> mem, EntityHandle entity, ComponentBase* comp);
	typedef void(*RemoveComponentFunc)(ComponentBase* comp);

    EntityHandle entity = nullptr;
    static V3API unsigned int registerType(CreateComponentFunc createfn, RemoveComponentFunc removefn, size_t size);

	static CreateComponentFunc getCreateFn(unsigned int id)
	{
		return std::get<0>(types[id]);
	};

	static RemoveComponentFunc getRemoveFn(unsigned int id)
	{
		return std::get<1>(types[id]);
	};

	static size_t getSize(unsigned int id)
	{
		return std::get<2>(types[id]);
	};

	static V3API std::vector<std::tuple<CreateComponentFunc, RemoveComponentFunc, size_t>> types;
};

template<typename T>
struct Component : public ComponentBase
{
	static const CreateComponentFunc create;
	static const RemoveComponentFunc remove;
	static const size_t size;

	static const unsigned int type;
};
template<typename T>
const unsigned int Component<T>::type = ComponentBase::registerType(CreateComponent<T>, RemoveComponent<T>, sizeof(T));
template<typename T>
const size_t Component<T>::size = sizeof(T);

template<typename C>
unsigned int CreateComponent(std::vector<unsigned int> mem, EntityHandle entity, ComponentBase* comp)
{
	unsigned int index = mem.size();
	mem.resize(index + C::size);
	C* c = new(&mem[index]) C(*(C*)comp);
	c->entity = entity;
	return index;
};
template<typename T>
const ComponentBase::CreateComponentFunc Component<T>::create = CreateComponent<T>;

template<typename C>
void RemoveComponent(ComponentBase* comp)
{
	C* c = (C*)comp;
	c->~C();
}
template<typename T>
const ComponentBase::RemoveComponentFunc Component<T>::remove = RemoveComponent<T>;