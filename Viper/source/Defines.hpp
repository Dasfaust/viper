#pragma once
#define NOMINMAX
#include <chrono>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "util/robin_hood.h"
#include <atomic>

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;
typedef double time_val;
typedef std::string uid;
typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef long long int64;

namespace Time
{
	typedef std::chrono::time_point<std::chrono::steady_clock> point;

	static const int64 nsDivisor = 1000000;
	
	inline static point now()
	{
		return std::chrono::high_resolution_clock::now();
	};

	inline static int64 since(point now, point past)
	{
		 return std::chrono::duration_cast<std::chrono::nanoseconds>(now - past).count();
	};

	inline static int64 since(point past)
	{
		return since(now(), past);
	};

	inline static float toMilliseconds(int64 nanoseconds)
	{
		return static_cast<float>(nanoseconds) / static_cast<float>(nsDivisor);
	};

	inline static float toSeconds(int64 nanoseconds)
	{
		return static_cast<float>(nanoseconds) / static_cast<float>(nsDivisor * 1000);
	};
};


#define umap(x, y) robin_hood::unordered_map<x, y>
#define set_atom(x, y, z) z __x = y; while (!x.compare_exchange_strong(__x, y))
#define distance2d(v1, v2) sqrt(pow(v2.x - v1.x, 2) + pow(v2.y - v1.y, 2) * 1.0f)