#pragma once
#include "../Defines.hpp"

namespace ecs
{
	struct Entity
	{
		uint64 id = 0;
		std::vector<uint32> components;
		bool skip = false;
	};
};