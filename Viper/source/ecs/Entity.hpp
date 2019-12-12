#pragma once
#include "../Defines.hpp"
#include <bitset>

namespace ecs
{
	struct Entity
	{
		uint64 id = 0;
		std::bitset<64> components;
		std::bitset<64> systems;
		bool skip = false;
	};
};