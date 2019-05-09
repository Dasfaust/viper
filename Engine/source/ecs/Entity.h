#pragma once
#include "Base.h"
#include <boost/container/flat_map.hpp>
#include <unordered_map>

namespace ECS
{
	struct Entity : ObjectBase
	{
		// unordered_map throws 'Iterator cannot be incremented' error here on count()
		// only if there are more than 1 component. ???
		boost::container::flat_map<uint8, uint32> components = boost::container::flat_map<uint8, uint32>();
	};
};