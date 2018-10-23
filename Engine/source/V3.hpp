#pragma once
#include "config/ConfigLayer.h"
#include "view/ViewLayer.h"

class V3
{
public:
	V3(std::string workingDir);
	~V3();

	ConfigLayer* getConfig();
	ViewLayer* getView();
private:
	std::shared_ptr<ConfigLayer> config;
	std::shared_ptr<ViewLayer> view;
};
