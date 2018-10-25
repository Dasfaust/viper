#pragma once
#include "../config/ConfigLayer.h"

class ViewLayer
{
public:
	ViewLayer(ConfigLayer* config);
	~ViewLayer();
private:
	ConfigLayer* config;
};

