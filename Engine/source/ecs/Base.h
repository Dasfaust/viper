#pragma once
#include <stdint.h>

typedef uint8_t uint8;
typedef uint32_t uint32;

namespace ECS
{
	struct ObjectBase
	{
		uint8 type_id;
		size_t type_size;
		uint32 index;

		ObjectBase* clone(std::vector<uint32>& heap, uint32 index)
		{
			return new(&heap[index]) ObjectBase(*this);
		};
	};
};