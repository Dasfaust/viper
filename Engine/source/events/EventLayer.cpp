#include "EventLayer.h"

EventLayer::EventLayer()
{
	onConfigChanged = std::make_shared<Event::OnConfigChanged>();
}

EventLayer::~EventLayer()
{
}

std::shared_ptr<Event::OnConfigChanged> EventLayer::getOnConfigChanged()
{
	return onConfigChanged;
}
