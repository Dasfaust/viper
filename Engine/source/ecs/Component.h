#pragma once
#include "Base.h"

namespace ECS
{
	struct Component : ObjectBase
	{
		ObjectBase* entity;
	};
};