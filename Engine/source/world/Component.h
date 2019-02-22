#pragma once
#include <vector>

typedef void* EntityHandle;
struct ComponentBase
{
    EntityHandle entity = nullptr;

    static unsigned int registerType();
};

typedef unsigned int(*CreateComponentFunc)(std::vector<short> mem, EntityHandle entity, ComponentBase* comp);
typedef void(*RemoveComponentFunc)(ComponentBase* comp);

template<typename T>
struct Component : public ComponentBase
{
	static const CreateComponentFunc create;
	static const RemoveComponentFunc remove;
	static const size_t size;

	static const unsigned int type;
};
template<typename T>
const unsigned int Component<T>::type = ComponentBase::registerType();
template<typename T>
const size_t Component<T>::size = sizeof(T);

template<typename C>
unsigned int CreateComponent(std::vector<short> mem, EntityHandle entity, ComponentBase* comp)
{
	unsigned int index = mem.size();
	mem.resize(index + C::size);
	C* c = new(&mem[index]) C(*(C*)comp);
	c->entity = entity;
	return index;
};
template<typename T>
const CreateComponentFunc Component<T>::create = CreateComponent<T>;

template<typename C>
void RemoveComponent(ComponentBase* comp)
{
	C* c = (C*)comp;
	c->~C();
}
template<typename T>
const RemoveComponentFunc Component<T>::remove = RemoveComponent<T>;