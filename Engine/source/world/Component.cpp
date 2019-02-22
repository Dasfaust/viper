#include "Component.h"

static unsigned int gComponentTypeIndex = 0;

unsigned int ComponentBase::registerType()
{
    return gComponentTypeIndex++;
}