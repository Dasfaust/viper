#pragma once
#include <stdint.h>

typedef uint8_t uint8;
typedef uint32_t uint32;

namespace ECS
{
	struct ObjectBase
	{
		uint8 type_id = 0;
		size_t type_size = 0;
		uint32 index = 0;

		ObjectBase* clone(std::vector<uint32>& heap, uint32 index)
		{
			return new(&heap[index]) ObjectBase(*this);
		};
	};
};