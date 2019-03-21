#pragma once
#include "../Logger.h"
#include "MemoryPool.h"
#include "../Tickable.h"

template<typename T>
class ObjectMap
{
public:
	inline void emplace(unsigned int pos, T& elem)
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

