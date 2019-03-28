#pragma once
#include "../Macros.h"
#include "../Logger.h"
#include "Component.h"
#include "Entity.h"
#include "System.h"
#include <boost/container/flat_map.hpp>

namespace ECS
{
	struct TestComponent : public Component
	{
		double val1 = 0;
	};

	struct TypeInfo
	{
		uint8 id;
		size_t size;
	};

	// Contains, handles all ECS functions
	class Container
	{
	public:
		Container()
		{
			resolveType<Entity>();

			/*debug("Creating entities...");
			std::vector<unsigned int> ents;
			for (unsigned int i = 0; i < 1000000; i++)
			{
				ents.push_back(createEntity());
			}

			debugf("Heap size: %d", heap[0].size());

			debug("Deleting entities...");
			for (unsigned int i = 0; i < 1000000; i++)
			{
				deleteEntity(getEntity(ents[i]));
			}

			heap[0].shrink_to_fit();
			debugf("Heap size: %d", heap[0].size());*/

			std::vector<uint32> ents;
			for (unsigned int i = 0; i < 10; i++)
			{
				ents.push_back(createEntity());
				uint32 c1 = createComponent<TestComponent>(getEntity(ents[i]));
				debugf("Component: %d", c1);
			}

			debugf("Entities: %d", heap[0].size() / resolveType<Entity>().size);
			debugf("Components: %d", heap[1].size() / resolveType<TestComponent>().size);

			uint32 e = createEntity();
			auto ei = getEntity(e);
			auto info = resolveType<TestComponent>();
			uint32 c = createComponent(ei, info.id, info.size);
			debugf("Component: %d", getComponent<TestComponent>(ei)->index);

			System* sys = createSystem([](double dt, Component* component, System* system, Container* container)
			{
				//debugf("Ticking component: %d", component->index);
				TestComponent* comp = reinterpret_cast<TestComponent*>(component);
				comp->val1 = rand();
			}, { resolveType<TestComponent>().id });
		};

		~Container()
		{
			for (auto kv : heap)
			{
				(&kv)->second.clear();
				(&kv)->second.shrink_to_fit();
			};
		};

		// Return index because pointer loses scope after another
		// entity is created. I can't even, Becky
		inline uint32 createEntity()
		{
			return instantiate<Entity>(resolveType<Entity>())->index;
		};

		inline Entity* getEntity(uint32 index)
		{
			return deref<Entity>(index);
		};

		inline void deleteEntity(Entity* entity)
		{
			for (auto kv : entity->components)
			{
				uint8 type = (&kv)->first;
				deleteComponent(type, (&kv)->second->index);
				heap[type].shrink_to_fit();
			}

			swapAndPop(entity);
		};

		inline void removeComponent(Entity* entity, Component* component)
		{
			entity->components.erase(component->type_id);
			deleteComponent(component);
		};

		template<typename T>
		uint32 createComponent(Entity* entity)
		{
			T* component = instantiate<T>(resolveType<T>());
			entity->components[component->type_id] = deref(component->type_id, component->index);
			component->entity = entity;
			return component->index;
		};

		uint32 createComponent(Entity* entity, uint8 type, size_t size)
		{
			Component* component = reinterpret_cast<Component*>(instantiate({ type, size }));
			entity->components[component->type_id] = deref(component->type_id, component->index);
			component->entity = entity;
			return component->index;
		};

		template<typename T>
		inline T* getComponent(Entity* entity)
		{
			return static_cast<T*>(entity->components[resolveType<T>().id]);
		};

		inline void deleteComponent(Component* component)
		{
			Entity* entity = static_cast<Entity*>(component->entity);
			size_t type = component->type_id;
			auto kv = swapAndPop(component);
			entity->components.erase(type);
		};

		inline void deleteComponent(uint8 type, uint32 index)
		{
			deleteComponent(static_cast<Component*>(deref(type, index)));
		};

		inline System* createSystem(void(*tickFunc)(double, Component*, System*, Container*), std::vector<uint8> types)
		{
			System* system = instantiate<System>(resolveType<System>());
			boost::container::flat_map<uint8, size_t> typeMap;
			for (uint8 i = 0; i < types.size(); i++)
			{
				typeMap[types[i]] = deref(types[i], 0)->type_size;
			}
			system->init(tickFunc, typeMap);
			systems.push_back(system);
			return static_cast<System*>(deref<System>(system->index));
		};

		inline std::vector<System*>& getSystems()
		{
			return systems;
		};

		inline std::vector<uint32>& getHeap(uint8 type)
		{
			return heap[type];
		};
	private:
		boost::container::flat_map<std::string, TypeInfo> typeIndex;
		boost::container::flat_map<uint8, std::vector<uint32>> heap;
		std::vector<System*> systems;

		template<typename T>
		inline TypeInfo& resolveType()
		{
			std::string type = std::string(typeid(T).name());
			if (!typeIndex.count(type))
			{
				uint8 id = typeIndex.size();
				size_t size = sizeof(T);
				typeIndex[type] = { id, size };

				// Create an instance to copy from for all future objects
				uint32 index =  allocate(typeIndex[type]);
				T* instance = new(&heap[id][index]) T();
				instance->type_id = id;
				instance->type_size = size;
			}
			return typeIndex[type];
		};

		inline uint32 allocate(TypeInfo info)
		{
			if (!heap.count(info.id))
			{
				heap[info.id] = std::vector<uint32>();
			}
			uint32 index = heap[info.id].size();
			heap[info.id].resize(index + info.size);
			return index;
		};

		template<typename T>
		inline T* instantiate(TypeInfo info)
		{
			uint32 index = allocate(info);
			T* instance = new(&heap[info.id][index]) T(*reinterpret_cast<T*>(&heap[info.id][0]));
			instance->index = index;
			return instance;
		};

		inline ObjectBase* instantiate(TypeInfo info)
		{
			uint32 index = allocate(info);
			ObjectBase* clone = deref(info.id, 0)->clone(heap[info.id], index);
			clone->index = index;
			return clone;
		};

		inline ObjectBase* deref(uint8 type, uint32 index)
		{
			return reinterpret_cast<ObjectBase*>(&heap[type][index]);
		};

		template<typename T>
		inline T* deref(uint32 index)
		{
			TypeInfo info = resolveType<T>();
			return reinterpret_cast<T*>(&heap[info.id][index]);
		}

		// Returns the new index and old index of the last element
		inline std::pair<uint32, uint32> swapAndPop(ObjectBase* obj)
		{
			uint8 type = obj->type_id;
			size_t size = obj->type_size;

			uint32 lastIndex = heap[type].size() - obj->type_size;
			uint32 index = obj->index;

			if (lastIndex == 0)
			{
				heap[type].resize(lastIndex + size);
			}
			else
			{
				ObjectBase* last = deref(obj->type_id, obj->type_size);

				memcpy(obj, last, size);
				heap[type].resize(lastIndex);
			}

			return std::make_pair(lastIndex, index);
		};
	};
};