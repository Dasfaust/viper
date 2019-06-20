#pragma once
#include <vector>
#include <unordered_map>
#include <typeindex>
#include "../Defines.hpp"

struct t_info
{
	uint32 id;
	size_t size;
	std::string name;
};

struct object
{
	uint32 id;
	t_info* type;
};

class StaticPool
{
public:
	umap(std::type_index, t_info*) types = umap(std::type_index, t_info*)();
	umap(uint32, std::vector<uint32>) heaps = umap(uint32, std::vector<uint32>)();
	umap(uint32, std::vector<uint32>) freeSlots = umap(uint32, std::vector<uint32>)();

	template<typename T>
	inline t_info* resolve()
	{
		std::type_index index = std::type_index(typeid(T));
		if (types.count(index))
		{
			return types[index];
		}
		uint32 id = (uint32)types.size();
		t_info* type = new t_info();
		type->id = id;
		type->size = sizeof(T);
		type->name = typeid(T).name();
		types[index] = type;

		uint32 obId = allocate(type);
		T* instance = new(&heaps[type->id][obId]) T();
		instance->id = obId;
		instance->type = types[index];

		return types[index];
	};

	inline t_info* getType(uint32 id)
	{
		t_info* type;
		for (auto&& kv : types)
		{
			if (kv.second->id == id)
			{
				type = kv.second;
			}
		}
		return type;
	};

	inline t_info* getType(std::type_index index)
	{
		return types[index];
	};

	inline uint32 allocate(t_info* type)
	{
		uint32 free;
		if (freeSlots.count(type->id) && !freeSlots[type->id].empty())
		{
			free = freeSlots[type->id][0];
			freeSlots[type->id].erase(freeSlots[type->id].begin());
		}
		else
		{
			free = (uint32)heaps[type->id].size();
			heaps[type->id].resize(free + type->size);
		}
		return free;
	};

	inline uint32 instantiate(t_info* type)
	{
		uint32 id = allocate(type);
		auto instance = new(&heaps[type->id][id]) object(*reinterpret_cast<object*>(&heaps[type->id][0]));
		instance->id = id;
		instance->type = type;
		return id;
	};

	inline object* get(t_info* type, uint32 id)
	{
		return reinterpret_cast<object*>(&heaps[type->id][id]);
	};

	template<typename T>
	inline T* get(uint32 id)
	{
		return reinterpret_cast<T*>(&heaps[resolve<T>()->id][id]);
	};

	inline object* create(t_info* type)
	{
		return get(type, instantiate(type));
	};

	template<typename T>
	inline T* create()
	{
		return reinterpret_cast<T*>(create(resolve<T>()));
	};

	inline void del(t_info* type, uint32 id)
	{
		freeSlots[type->id].push_back(id);
	};

	template<typename T>
	inline void del(uint32 id)
	{
		freeSlots[resolve<T>()->id].push_back(id);
	};

	inline std::vector<object*> getAll(t_info* type)
	{
		std::vector<object*> vec;
		for (uint32 i = (uint32)type->size; i < heaps[type->id].size(); i += (uint32)type->size)
		{
			vec.push_back(get(type, i));
		}
		return vec;
	};

	template<typename T>
	inline std::vector<T*> getAll()
	{
		auto type = resolve<T>();
		std::vector<T*> vec;
		for (uint32 i = type->size; i < heaps[type->id].size(); i += type->size)
		{
			vec.push_back(reinterpret_cast<T*>(get(type, i)));
		}
		return vec;
	};

	inline void purge()
	{
		for (auto&& kv : heaps)
		{
			kv.second.clear();
			kv.second.shrink_to_fit();
		}
		heaps.clear();
		freeSlots.clear();
		for (auto&& kv : types)
		{
			delete kv.second;
		}
		heaps.clear();
	};
};