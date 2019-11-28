#pragma once
#define NOMINMAX
#include <unordered_map>
#include <boost/container/flat_map.hpp>
#include <chrono>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <set>

typedef unsigned long long int uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef double time_val;
typedef boost::uuids::uuid uid;
typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::mat3 mat3;
typedef glm::mat4 mat4;

#define flatmap(x, y) boost::container::flat_map<x, y>
#define umap(x, y) std::unordered_map<x, y>
#define tnowms() static_cast<time_val>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count())
#define tnowns() static_cast<time_val>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count())
#define set_atom(x, y, z) z __x = y; while (!x.compare_exchange_strong(__x, y))
#define timesince(x) (tnowns() - x) / 1000000.0