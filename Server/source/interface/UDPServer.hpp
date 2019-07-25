#pragma once
#include "Defines.hpp"
#include "interface/Module.hpp"
#include "util/JSON.hpp"

typedef unsigned long long sock;

class UDPServer : public Module
{
public:
	char bufferSize = 1024;
};