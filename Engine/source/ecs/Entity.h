#pragma once
#include "Base.h"
#include <boost/container/flat_map.hpp>

namespace ECS
{
	struct Entity : ObjectBase
	{
		boost::container::flat_map<uint8, ObjectBase*> components;
	};
};