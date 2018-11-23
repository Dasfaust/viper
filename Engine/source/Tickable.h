#pragma once
#include <tbb/concurrent_vector.h>
#include <memory>

class Tickable
{
public:
	typedef tbb::concurrent_vector<std::shared_ptr<Tickable>> TicArray;

	virtual ~Tickable() { };
	virtual void tick() { };
};