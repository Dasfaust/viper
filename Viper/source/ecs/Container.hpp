#pragma once
#include "../interface/Modular.hpp"
#include "System.hpp"
#include <set>

namespace ecs
{
	namespace ComponentFlags
	{
		enum ComponentFlag
		{
			E_REQUIRED,
			E_OPTIONAL,
			E_SKIP
		};
	};
	
	static thread_local uint32 componentIndex = 0;
	
	template<typename T>
	struct ComponentIDs
	{
		inline static uint32 ID = 0;
	};

	struct ComponentMeta
	{
		uint32 id = 0;
		std::shared_ptr<void> object;
		size_t size;
		size_t tSize;
		void*(*instantiate)(std::vector<uint32>& heap, uint64 index, ComponentMeta& self);
	};
	
	class Container : public Module, public Modular
	{
	public:
		uint32 blockSizeMb = 8;
		bool async = false;
		uint32 threads = 2;

		uint32 blocksAllocated = 0;
		size_t entitySize;
		std::vector<ComponentMeta> componentData;
		std::vector<std::set<std::pair<uint32, ComponentFlags::ComponentFlag>>> systemTypes;
		std::vector<std::shared_ptr<System>> systems;
		inline static thread_local std::vector<size_t> offsets;
		std::vector<uint32> heap;
		std::vector<uint64> deletions;
		bool firstTick = true;

		void onStart() override;
		void onShutdown() override;
		uint64 getNextEntityId();
		uint64 makeEntity(std::set<uint32> comps);
		void updateSystemsCache(uint64 ent);
		Entity* getEntity(uint64 id);
		void deleteEntity(uint64 id);
		void purge();
		void allocateNewBlock();
		void onTick() override;

		template<typename T>
		void registerComponent()
		{
			uint32 id = componentIndex++;
			ComponentIDs<T>::ID = id;
			if (componentData.size() < id + 1) componentData.resize(id + 1);
			componentData[id] = {
				id,
				std::make_shared<T>(),
				sizeof(T),
				sizeof(T) + sizeof(std::atomic_flag),
				[](std::vector<uint32>& heap, uint64 index, ComponentMeta& self) -> void*
				{
					new(&heap[index + self.size]) std::atomic_flag();
					return new(&heap[index]) T(*std::static_pointer_cast<T>(self.object));
				}
			};
		};

		template<typename T>
		T* getComponent(uint64 id)
		{
			return reinterpret_cast<T*>(&heap[id * entitySize + offsets[ComponentIDs<T>::ID]]);
		};

		void* getComponent(uint64 eid, uint8 cid)
		{
			return &heap[eid * entitySize + offsets[cid]];
		};

		template<typename T>
		std::atomic_flag* getFlag(uint64 id)
		{
			auto compId = ComponentIDs<T>::ID;
			return reinterpret_cast<std::atomic_flag*>(&heap[id * entitySize + offsets[compId] + componentData[compId].size]);
		};

		std::atomic_flag* getFlag(uint64 eid, uint8 cid)
		{
			return reinterpret_cast<std::atomic_flag*>(&heap[eid * entitySize + offsets[cid] + componentData[cid].size]);
		};

		template<typename S>
		std::shared_ptr<S> initSystem(std::set<std::pair<uint32, ComponentFlags::ComponentFlag>> types)
		{
			auto id = systemTypes.size();
			systemTypes.push_back(types);
			systems.resize(id + 1);
			systems[id] = std::make_shared<S>();
			return std::reinterpret_pointer_cast<S>(systems[id]);
		};

		std::shared_ptr<System> initSystem(std::set<std::pair<uint32, ComponentFlags::ComponentFlag>> types, void(*updateEntity)(Entity* entity, std::shared_ptr<System> self, float dt))
		{
			auto system = initSystem<System>(types);
			system->updateEntity = updateEntity;
			return system;
		};
	};

	template<typename T>
	inline static T* shift(Entity* ent)
	{
		return reinterpret_cast<T*>(reinterpret_cast<uint32*>(ent) + Container::offsets[ecs::ComponentIDs<T>::ID]);
	};

	template<typename T>
	inline static std::atomic_flag* shiftFlag(Entity* ent)
	{
		return reinterpret_cast<T*>(reinterpret_cast<uint32*>(ent) + Container::offsets[ecs::ComponentIDs<T>::ID] + sizeof(T));
	};
};