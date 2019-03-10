#pragma once
#include "../Logger.h"
#include "MemoryPool.h"
#include "../Tickable.h"
#include <unordered_map>
#include <functional>

class ECSBase
{
public:
	unsigned int id;
	unsigned int parent;
	unsigned int type_id = 0;
	std::function<void(ECSBase*)> del_func;
	std::function<void(unsigned int)> tick_func;
	bool is_client = false;

	virtual ~ECSBase()
	{
	
	};
};

namespace ECS
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
};

template<typename Self>
class Component : public ECSBase
{
public:
	static const size_t size;
};
template<typename Self>
const size_t Component<Self>::size = sizeof(Self);

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
};