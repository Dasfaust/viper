#pragma once
#include "Entity.hpp"
#include "System.hpp"
#include "../interface/Modular.hpp"
#include "../interface/Threadable.hpp"
#include <atomic>

namespace ecs
{
	static uint32 componentIndex = 0;
	
	template<typename T>
	struct ComponentIDs
	{
		static uint32 ID;
	};
	template<typename T>
	uint32 ComponentIDs<T>::ID = componentIndex++;

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
		std::vector<std::shared_ptr<System>> systems;
		std::vector<size_t> offsets;
		std::vector<size_t> queueOffsets;
		std::vector<uint32> heap;
		std::vector<uint64> deletions;

		void onStart() override;
		void onShutdown() override;
		uint64 getNextEntityId();
		uint64 makeEntity(std::vector<uint32> comps);
		Entity* getEntity(uint64 id);
		void deleteEntity(uint64 id);
		void purge();
		void allocateNewBlock();
		void onTick() override;

		template<typename T>
		void registerComponent()
		{
			uint32 id = ComponentIDs<T>::ID;
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

		void* getComponent(uint64 eid, uint32 cid)
		{
			return &heap[eid * entitySize + offsets[cid]];
		};

		template<typename T>
		std::atomic_flag* getFlag(uint64 id)
		{
			auto compId = ComponentIDs<T>::ID;
			return reinterpret_cast<std::atomic_flag*>(&heap[id * entitySize + offsets[compId] + componentData[compId].size]);
		};

		std::atomic_flag* getFlag(uint64 eid, uint32 cid)
		{
			return reinterpret_cast<std::atomic_flag*>(&heap[eid * entitySize + offsets[cid] + componentData[cid].size]);
		};

		template<typename T, typename S>
		std::shared_ptr<S> initSystem()
		{
			auto compId = ComponentIDs<T>::ID;
			if (systems.size() <= compId)
			{
				systems.resize(compId + 1);
			}
			systems[compId] = std::make_shared<S>();
			return systems[compId];
		};

		template<typename T>
		std::shared_ptr<System> initSystem(void(*updateEntity)(Entity* entity, void* component, std::shared_ptr<System> self))
		{
			auto system = initSystem<T, System>();
			system->updateEntity = updateEntity;
			return system;
		};
	};
};