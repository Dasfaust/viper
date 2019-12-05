#pragma once
#include "../Defines.hpp"

namespace ecs
{
	struct Entity
	{
		uint64 id = 0;
		std::array<void*, 32> componentPointers;
		std::array<int, 16> systems;
		bool skip = false;
	};
};