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

			/*std::vector<uint32> ents;
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
			}, { resolveType<TestComponent>().id });*/
		};

		~Container()
		{
			for (auto kv : heap)
			{
				(&kv)->second.clear();
				(&kv)->second.shrink_to_fit();
			};
		};

		template<typename T>
		inline TypeInfo& resolveType()
		{
			std::string type = std::string(typeid(T).name());
			if (!typeCache.count(type))
			{
				uint8 id = (uint8)typeCache.size();
				size_t size = sizeof(T);
				typeCache[type] = id;
				typeIndex[id] = { id, size };

				// Create an instance to copy from for all future objects
				uint32 index = allocate(typeIndex[id]);
				T* instance = new(&heap[id][index]) T();
				instance->type_id = id;
				instance->type_size = size;
			}
			return typeIndex[typeCache[type]];
		};

		inline TypeInfo& resolveType(uint8 id)
		{
			return typeIndex[id];
		};

		// Return index because pointer loses scope after another
		// entity is created. I can't even, Becky
		inline uint32 createEntity()
		{
			auto ent = instantiate<Entity>(resolveType<Entity>());
			return ent->index;
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
				deleteComponent(reinterpret_cast<Component*>((type, (&kv)->second)));
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
			entity->components[component->type_id] = component->index;
			component->entity = entity->index;
			return component->index;
		};

		uint32 createComponent(Entity* entity, uint8 type, size_t size)
		{
			Component* component = reinterpret_cast<Component*>(deref(type, instantiate({ type, size })));
			entity->components[component->type_id] = component->index;
			component->entity = entity->index;
			return component->index;
		};

		template<typename T>
		inline T* getComponent(Entity* entity)
		{
			auto type = resolveType<T>();
			return reinterpret_cast<T*>(deref(type.id, entity->components[type.id]));
		};

		inline Component* getComponent(Entity* entity, uint8 type)
		{
			return static_cast<Component*>(deref(type, entity->components[type]));
		};

		inline void deleteComponent(Component* component)
		{
			Entity* entity = getEntity(component->entity);
			uint8 type = component->type_id;
			auto kv = swapAndPop(component);
			entity->components.erase(type);
		};

		inline void deleteComponent(uint8 type, uint32 index)
		{
			deleteComponent(static_cast<Component*>(deref(type, index)));
		};

		inline System* createSystem(TypeInfo type, uint32 index, V3* v3, World* world)
		{
			uint32 id = instantiate(type);
			System* system = reinterpret_cast<System*>(deref(type.id, id));
			system->index = id;
			system->preinit(v3);
			system->init(this, world);
			debugf("System created: type: %d, size %d, index %d", system->type_id, system->type_size, system->index);
			systems.resize(index + 1);
			systems[index] = reinterpret_cast<System*>(deref(system->type_id, system->index));
			return reinterpret_cast<System*>(deref(system->type_id, system->index));
		};

		template<typename T>
		inline T* createSystem(V3* v3, int position = -1)
		{
			T* system = instantiate<T>(resolveType<T>(), false);
			system->preinit(v3);
			system->init();
			systems.push_back(static_cast<T*>(deref<T>(system->index)));
			return static_cast<T*>(deref<T>(system->index));
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
		boost::container::flat_map<std::string, uint8> typeCache;
		boost::container::flat_map<uint8, TypeInfo> typeIndex;
		boost::container::flat_map<uint8, std::vector<uint32>> heap;
		std::vector<System*> systems;

		inline uint32 allocate(TypeInfo info)
		{
			if (!heap.count(info.id))
			{
				heap[info.id] = std::vector<uint32>();
			}
			uint32 index = (uint32)heap[info.id].size();
			heap[info.id].resize(index + info.size);
			return index;
		};

		template<typename T>
		inline T* instantiate(TypeInfo info, bool copy = true)
		{
			uint32 index = allocate(info);
			T* instance;
			if (copy)
			{
				instance = new(&heap[info.id][index]) T(*reinterpret_cast<T*>(&heap[info.id][0]));
			}
			else
			{
				instance = new(&heap[info.id][index]) T();
			}
			instance->index = index;
			return instance;
		};

		inline uint32 instantiate(TypeInfo info)
		{
			uint32 index = allocate(info);
			deref(info.id, 0)->clone(heap[info.id], index)->index = index;
			//debugf("Cloning type %d, new index %d", info.id, index);
			return index;
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

			uint32 lastIndex = (uint32)(heap[type].size() - obj->type_size);
			uint32 index = obj->index;

			if (lastIndex == 0)
			{
				heap[type].resize(lastIndex + size);
			}
			else
			{
				ObjectBase* last = deref(obj->type_id, obj->index);

				memcpy(obj, last, size);
				heap[type].resize(lastIndex);
			}

			return std::make_pair(lastIndex, index);
		};
	};
};