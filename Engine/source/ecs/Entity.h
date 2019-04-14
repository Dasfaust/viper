#pragma once
#include "Base.h"
#include <boost/container/flat_map.hpp>
#include <unordered_map>

namespace ECS
{
	struct Entity : ObjectBase
	{
		std::unordered_map<uint8, uint32> components;
	};
};