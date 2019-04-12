#pragma once
#include "Event.h"
#include "../Module.h"
#include "boost/container/flat_map.hpp"

class EventLayer : public Module
{
public:
private:
	boost::container::flat_map<unsigned int, std::vector<unsigned int>> heap;
};