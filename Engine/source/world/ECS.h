#pragma once
#include "../Logger.h"
#include "MemoryPool.h"
#include "../Tickable.h"
#include <functional>

template<typename T>
class VecMap
{
public:
	inline void emplace(unsigned int pos, T elem)
	{
		if (data.size() < pos + 1)
		{
			data.resize(pos + 1);
		}
		data[pos] = elem;
	};

	inline void remove(unsigned int pos)
	{
		/*if (data.size() >= pos)
		{
			data.erase(data.begin() + pos);
		}*/
		data.erase(data.begin() + pos);
	};

	inline T get(unsigned int id)
	{
		return data[id];
	};

	inline bool exists(unsigned int id)
	{
		if (data.size() >= id + 1)
		{
			return sizeof(data[id]) > 0;
		}
		return false;
	};

	inline std::vector<T> getRaw()
	{
		return data;
	};
private:
	std::vector<T> data;
};

template<typename T>
struct type_id
{
	static const size_t type;
};
template<typename T>
const size_t type_id<T>::type = rand();

struct Base
{
	unsigned int id;
	unsigned int parent;
};

struct Component : public Base
{
	unsigned int type;
};

class World;

class SystemBase : public Base
{
public:
	unsigned int type;

	virtual void tickInternal(Component* comp) { };
};

template<typename T>
class System : public SystemBase
{
public:
	World* world;
	
	void tickInternal(Component* comp) override
	{
		tick(static_cast<T*>(comp));
	};

	virtual void tick(T* comp) { };
};

class PoolBase : public Base
{
public:
	virtual void destroy(Component* elem) { };
};

template<typename T>
class PoolWrapper : public PoolBase
{
public:
	inline T* make()
	{
		return pool.newElement();
	};

	inline void destroy(Component* elem) override
	{
		debugf("Destroying component: %d", elem->id);
		pool.deleteElement(static_cast<T*>(elem));
	};
private:
	MemoryPool<T> pool;
};

class ECS
{
public:
	template<typename T>
	inline unsigned int getType()
	{
		auto str = std::string(typeid(T).name());
		if (typeIndex.count(str))
		{
			return typeIndex[str];
		}
		typeIndex[str] = typeIndex.size();
		return typeIndex[str];
	};

	template<typename T>
	inline std::shared_ptr<PoolWrapper<T>> getPool()
	{
		unsigned int type = getType<T>();
		if (wrappers.count(type))
		{
			return std::static_pointer_cast<PoolWrapper<T>>(wrappers[type]);
		}
		wrappers[type] = std::make_shared<PoolWrapper<T>>();
		return std::static_pointer_cast<PoolWrapper<T>>(wrappers[type]);
	};

	inline unsigned int assign()
	{
		if (freeObjectIds.size() > 0)
		{
			unsigned int id = freeObjectIds[0];
			std::iter_swap(freeObjectIds.begin(), freeObjectIds.end() - 1);
			freeObjectIds.pop_back();
			return id;
		}
		return objectIndex++;
	};

	inline unsigned int makeEntity()
	{
		return assign();
	};

	template<typename T>
	inline T* makeComponent()
	{
		unsigned int type = getType<T>();
		auto pool = getPool<T>();
		unsigned int id = assign();

		auto comp = pool->make();
		comp->id = id;
		comp->type = type;

		components[type][id] = comp;
		componentsVec[type].emplace_back(comp);

		return static_cast<T*>(components[type][id]);
	};

	template<typename T>
	inline T* getComponent(unsigned int id)
	{
		unsigned int type = getType<T>();
		return static_cast<T*>(components[type][id]);
	};

	inline Component* getComponent(unsigned int type, unsigned int id)
	{
		return components[type][id];
	};

	template<typename T>
	inline std::vector<T*> getComponents()
	{
		unsigned int type = getType<T>();

		std::vector<T*> vec;
		if (components.count(type))
		{
			for (auto kv : components[type])
			{
				vec.emplace_back(static_cast<T*>((&kv)->second));
			}
		}

		return vec;
	};

	inline std::vector<Component*> getComponents(unsigned int type)
	{
		if (componentsVec.count(type))
		{
			return componentsVec[type];
		}

		std::vector<Component*> vec;
		return vec;
	};

	template<typename T>
	inline void attach(unsigned int entity, unsigned int id)
	{
		auto comp = getComponent<T>(id);
		comp->parent = id;

		entities[entity][comp->type] = comp;
	};

	inline std::unordered_map<unsigned int, Component*> getEntityComponents(unsigned int entity)
	{
		if (entities.count(entity))
		{
			return entities[entity];
		}
		std::unordered_map<unsigned int, Component*> map;
		return map;
	};

	Component* getEntityComponent(unsigned int entity, unsigned int type)
	{
		return entities[entity][type];
	}

	template<typename T>
	T* getEntityComponent(unsigned int entity)
	{
		return static_cast<T*>(entities[entity][type][0]);
	}

	template<typename T, typename C>
	inline std::shared_ptr<T> makeSystem(World* instance)
	{
		unsigned int type = getType<C>();
		auto sys = std::make_shared<T>();
		sys->id = systemIndex++;
		sys->type = type;
		sys->world = instance;
		systems[type].emplace_back(sys);
		return sys;
	};

	inline std::unordered_map<unsigned int, std::vector<std::shared_ptr<SystemBase>>> getSystems()
	{
		return systems;
	};

	inline void deleteEntity(unsigned int id)
	{
		for (auto kv : entities[id])
		{
			auto comp = (&kv)->second;
			auto pool = std::static_pointer_cast<PoolBase>(wrappers[comp->type]);

			pool->destroy(comp);
			components[comp->type].erase(comp->id);

			std::vector<Component*>::iterator it;
			for (it = componentsVec[comp->type].begin(); it != componentsVec[comp->type].end(); it++)
			{
				if ((*it)->id == comp->id)
				{
					componentsVec[comp->type].erase(it);
				}
			}
		}
		entities.erase(id);
	};

	inline void purge()
	{
		for (auto kv : entities)
		{
			deleteEntity((&kv)->first);
		}
	};
private:
	unsigned int systemIndex = 0;
	unsigned int objectIndex = 0;
	std::vector<unsigned int> freeObjectIds;
	std::unordered_map<std::string, unsigned int> typeIndex;

	std::unordered_map<unsigned int, std::unordered_map<unsigned int, Component*>> entities;
	std::unordered_map<unsigned int, std::unordered_map<unsigned int, Component*>> components;
	std::unordered_map<unsigned int, std::vector<Component*>> componentsVec;
	std::unordered_map<unsigned int, std::shared_ptr<PoolBase>> wrappers;
	std::unordered_map<unsigned int, std::vector<std::shared_ptr<SystemBase>>> systems;
};