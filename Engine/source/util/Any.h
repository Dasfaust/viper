#pragma once
#include "glm/glm.hpp"
#include "boost/uuid/uuid.hpp"

struct Any
{
	int int_val;
	double dub_val;
	float float_val;
	bool bool_val;
	glm::vec2 vec2_val;
	glm::vec3 vec3_val;
	boost::uuids::uuid uuid_val;
};

namespace any
{
	int i(Any& a)
	{
		return a.int_val;
	};

	double d(Any& a)
	{
		return a.dub_val;
	};

	float f(Any& a)
	{
		return a.float_val;
	};

	bool b(Any& a)
	{
		return a.bool_val;
	};

	glm::vec2 vec2(Any& a)
	{
		return a.vec2_val;
	};

	glm::vec3 vec3(Any& a)
	{
		return a.vec3_val;
	};

	boost::uuids::uuid uid(Any& a)
	{
		return a.uuid_val;
	};

	Any make(int i)
	{
		Any a = { };
		a.int_val = i;
		return a;
	};

	Any make(double d)
	{
		Any a = { };
		a.dub_val = d;
		return a;
	};

	Any make(float f)
	{
		Any a = { };
		a.float_val = f;
		return a;
	};

	Any make(bool b)
	{
		Any a = { };
		a.bool_val = b;
		return a;
	};

	Any make(glm::vec2 v)
	{
		Any a = { };
		a.vec2_val = v;
		return a;
	};

	Any make(glm::vec3 v)
	{
		Any a = { };
		a.vec3_val = v;
		return a;
	};

	Any make(boost::uuids::uuid u)
	{
		Any a = { };
		a.uuid_val = u;
		return a;
	};
}