#pragma once
#include "Base.h"

namespace ECS
{
	struct Component : ObjectBase
	{
		uint32 entity = 0;
	};
};