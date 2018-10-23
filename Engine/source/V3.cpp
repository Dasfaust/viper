#include "V3.hpp"
#include <memory>

V3::V3(std::string workingDir)
{
	config = std::make_shared<ConfigLayer>(workingDir);
	view = std::make_shared<ViewLayer>(config.get());
}

V3::~V3()
{

}

ConfigLayer* V3::getConfig()
{
	return config.get();
}

ViewLayer* V3::getView()
{
	return view.get();
}
