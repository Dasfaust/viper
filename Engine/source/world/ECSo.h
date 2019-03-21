#pragma once
#include "../Logger.h"
#include "MemoryPool.h"
#include "../Tickable.h"
#include <unordered_map>
#include <functional>

template<typename T>
class VecMap
{
public:
	inline void emplace(unsigned int pos, T elem)
	{
		if (data.size() < pos)
		{
			data.resize(pos);
		}
		data[pos] = elem;
	};

	inline void remove(unsigned int pos)
	{
		if (data.size() >= pos)
		{
			data.erase(data.begin(), pos);
		}
	};

	inline T get(unsigned int id)
	{
		return data[id];
	};
private:
	std::vector<T> data;
};

class ECSBase
{
public:
	unsigned int id;
	unsigned int type = 0;
	//std::function<void(ECSBase*)> del_func;
	//std::function<void(unsigned int)> tick_func;

	virtual ~ECSBase()
	{
	
	};
};

template<typename Self>
class Component : public ECSBase
{
public:
};

class ECS;

template<typename Comp>
class ComponentTicker
{
public:
	virtual void tick(Comp* comp)
	{

	};
};

template<typename Comp>
class System : public ECSBase
{
public:
	unsigned int type;
	MemoryPool<Comp> heap;
	std::shared_ptr<ComponentTicker<Comp>> ticker;

	static void tickInternal(unsigned int id)
	{
		static_cast<System<Comp>*>(ECS::systems[type_id])->tick(id);
	};

	inline void tick(unsigned int id)
	{
		ticker->tick(static_cast<Comp*>(ECS::components[id]));
	};

	static void deleteComponentInternal(ECSBase* component)
	{
		unsigned int id = component->id;
		if (ECS::compToEnt.count(id))
		{
			ECS::compToEnt.erase(id);
		}
		ECS::components.erase(id);
		ECS::componentIds.erase(std::remove(ECS::componentIds.begin(), ECS::componentIds.end(), id), ECS::componentIds.end());
		static_cast<System<Comp>*>(ECS::systems[component->type_id])->heap.deleteElement(static_cast<Comp*>(component));
	}

	void deleteComponent(ECSBase* component)
	{
		unsigned int id = component->id;
		if (ECS::compToEnt.count(id))
		{
			for (auto ent : ECS::compToEnt[id])
			{
				if (ECS::entToComp.count(ent->id))
				{
					auto list = ECS::entToComp[ent->id];
					std::iter_swap(list.begin(), list.end() - 1);
					list.pop_back();
					ECS::entToComp[ent->id] = list;
				}
			}
		}
		deleteComponentInternal(component);
	};

	inline Comp* makeComponent()
	{
		unsigned int id = ECS::assign();
		auto comp = heap.newElement();
		comp->id = id;
		comp->type_id = type_id;
		comp->del_func = deleteComponentInternal;
		comp->tick_func = tickInternal;
		ECS::components[id] = comp;
		ECS::componentIds.push_back(id);
		return static_cast<Comp*>(ECS::components[id]);
	};

	inline Comp* getComponent(unsigned int id)
	{
		if (ECS::components.count(id))
		{
			return static_cast<Comp*>(ECS::components[id]);
		}
		else
		{
			critf("ECS was asked for component that doesn't exist: %d", id);
			return nullptr;
		}
	};
};

class ECS
{
public:
	unsigned int objectIndex = 0;
	unsigned int typeIndex = 0;
	std::vector<unsigned int> freeObjectIds;

	// Component id, component
	std::unordered_map<unsigned int, ECSBase*> components;
	// Component type, ids
	std::unordered_map<unsigned int, std::vector<unsigned int>> componentsById;
	// Entity id, component type, ids
	std::unordered_map<unsigned int, std::unordered_map<unsigned int, std::vector<unsigned int>>> entities;
	// Type id, system
	std::unordered_map<unsigned int, ECSBase*> systems;

	unsigned int assign()
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


	unsigned int makeEntity()
	{
		return assign();
	};

	void addToEntity(unsigned int entity, ECSBase* component)
	{
		entities[entity][component->type].push_back(component->id);
	};

	void addToEntity(unsigned int entity, std::vector<ECSBase*> components)
	{
		for (auto comp : components)
		{
			addToEntity(entity, comp);
		}
	};

	std::vector<ECSBase*> getEntityComponents(unsigned int entity)
	{
		std::vector<ECSBase*> ret;
		for (auto kv : entities[entity])
		{
			ret.emplace_back((&kv)->second);
		}
		return ret;
	};

	template<typename T>
	std::vector<T*> getEntityComponentsByType(unsigned int entity)
	{
		std::vector<T*> ret;
		for (auto kv : entities[entity])
		{
			auto comp = (&kv)->second;
			if (dynamic_cast<T*>(comp) != nullptr)
			{
				ret.emplace_back(static_cast<T*>(comp));
			}
		}
		return &ret;
	};

	template<typename T>
	T* makeComponent()
	{
		unsigned int id = assign();
	};

	template<typename T>
	System<T> getSystem()
	{
		System<T>* system;
		for (auto kv : systems)
		{
			auto sys = (&kv)->second;
			if (dynamic_cast<System<T>*>(sys) != nullptr)
			{
				system = static_cast<System<T>*>(sys);
			}
		}
		return system;
	};

	template<typename T>
	T* getComponent(unsigned int id)
	{
		auto system = getSystem<T>();
		return static_cast<T*>(components[system->type][id]);
	};

	template<typename T>
	std::vector<T*> getComponentsByType()
	{
		std::vector<T*> ret;
		auto system = getSystem<T>();
		for (auto kv : components[system->type])
		{
			ret.emplace_back(static_cast<T*>(&(kv)->second));
		}
		return &ret;
	};

	void tickComponent(unsigned int id)
	{
		systems[components[id]]
	};
};

/*namespace ECS
{
	static unsigned int nextId = 0;
	static unsigned int nextTypeId = 1;

	static MemoryPool<ECSBase> entityHeap;
	static std::vector<unsigned int> freeIds;
	static std::unordered_map<unsigned int, ECSBase*> entities;
	static std::unordered_map<unsigned int, std::vector<ECSBase*>> entToComp;
	static std::unordered_map<unsigned int, ECSBase*> components;
	static std::vector<unsigned int> componentIds;
	static std::unordered_map<unsigned int, std::vector<ECSBase*>> compToEnt;
	static std::unordered_map<unsigned int, ECSBase*> systems;

	static void init()
	{
		
	};

	static unsigned int assign()
	{
		if (freeIds.size() > 0)
		{
			unsigned int id = freeIds[0];
			std::iter_swap(freeIds.begin(), freeIds.end() - 1);
			freeIds.pop_back();
			return id;
		}
		return nextId++;
	};

	static ECSBase* makeEntity()
	{
		unsigned int id = assign();
		entities[id] = entityHeap.newElement();
		entities[id]->id = id;
		return ECS::entities[id];
	};

	static ECSBase* getEntity(unsigned int id)
	{
		if (entities.count(id))
		{
			return entities[id];
		}
		else
		{
			critf("ECS was asked for entity that doesn't exist: %d", id);
			return nullptr;
		}
	};

	template<typename T>
	static void addComponentToEntity(ECSBase* ent, T* comp)
	{
		entToComp[ent->id].emplace_back(comp);
		compToEnt[comp->id].emplace_back(ent);
	};

	static std::vector<ECSBase*> getEntityComponents(ECSBase* ent)
	{
		if (entToComp.count(ent->id))
		{
			return entToComp[ent->id];
		}
		return std::vector<ECSBase*>();
	};

	static void deleteEntity(ECSBase* ent)
	{
		unsigned int id = ent->id;
		if (entities.count(id))
		{
			freeIds.push_back(id);
			entities.erase(id);
			if (entToComp.count(id))
			{
				for (auto comp : entToComp[id])
				{
					comp->del_func(comp);
				}
			}
			entToComp.erase(id);
			entityHeap.deleteElement(ent);
		}
		else
		{
			critf("ECS was asked for entity that doesn't exist: %d", id);
		}
	}

	static void purge()
	{
		for (auto kv : entities)
		{
			deleteEntity(kv.second);
		}
	};
};*/

/*template<typename Self>
class Component : public ECSBase
{
public:
};

template<typename Comp>
class ComponentTicker
{
public:
	virtual void tick(Comp* comp)
	{

	};
};

template<typename Comp>
class System : public ECSBase
{
public:
	static const unsigned int type_id;
	MemoryPool<Comp> heap;
	std::shared_ptr<ComponentTicker<Comp>> ticker;

	static void tickInternal(unsigned int id)
	{
		static_cast<System<Comp>*>(ECS::systems[type_id])->tick(id);
	};

	inline void tick(unsigned int id)
	{
		ticker->tick(static_cast<Comp*>(ECS::components[id]));
	};

	static void deleteComponentInternal(ECSBase* component)
	{
		unsigned int id = component->id;
		if (ECS::compToEnt.count(id))
		{
			ECS::compToEnt.erase(id);
		}
		ECS::components.erase(id);
		ECS::componentIds.erase(std::remove(ECS::componentIds.begin(), ECS::componentIds.end(), id), ECS::componentIds.end());
		static_cast<System<Comp>*>(ECS::systems[component->type_id])->heap.deleteElement(static_cast<Comp*>(component));
	}

	void deleteComponent(ECSBase* component)
	{
		unsigned int id = component->id;
		if (ECS::compToEnt.count(id))
		{
			for (auto ent : ECS::compToEnt[id])
			{
				if (ECS::entToComp.count(ent->id))
				{
					auto list = ECS::entToComp[ent->id];
					std::iter_swap(list.begin(), list.end() - 1);
					list.pop_back();
					ECS::entToComp[ent->id] = list;
				}
			}
		}
		deleteComponentInternal(component);
	};

	inline Comp* makeComponent()
	{
		unsigned int id = ECS::assign();
		auto comp = heap.newElement();
		comp->id = id;
		comp->type_id = type_id;
		comp->del_func = deleteComponentInternal;
		comp->tick_func = tickInternal;
		ECS::components[id] = comp;
		ECS::componentIds.push_back(id);
		return static_cast<Comp*>(ECS::components[id]);
	};

	inline Comp* getComponent(unsigned int id)
	{
		if (ECS::components.count(id))
		{
			return static_cast<Comp*>(ECS::components[id]);
		}
		else
		{
			critf("ECS was asked for component that doesn't exist: %d", id);
			return nullptr;
		}
	};
};
template<typename Comp>
const unsigned int System<Comp>::type_id = ECS::nextTypeId++;

namespace ECS
{
	template<typename T>
	static System<T>* makeSystem(std::shared_ptr<ComponentTicker<T>> ticker)
	{
		auto sys = new System<T>();
		sys->ticker = ticker;
		sys->tick_func = System<T>::tickInternal;
		systems[sys->type_id] = sys;
		return static_cast<System<T>*>(ECS::systems[sys->type_id]);
	};
};*/