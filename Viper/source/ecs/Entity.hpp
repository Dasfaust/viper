#pragma once
#include "../Defines.hpp"

namespace ecs
{
	struct Entity
	{
		uint64 id = 0;
		std::array<void*, 32> componentPointers;
		bool skip = false;
	};
};