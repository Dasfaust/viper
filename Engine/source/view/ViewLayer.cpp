#include "ViewLayer.h"
#include "../Logger.h"

ViewLayer::ViewLayer(ConfigLayer* config)
{
	this->config = config;
	info("ViewLayer init");
}

ViewLayer::~ViewLayer()
{
	info("ViewLayer destroyed")
}
