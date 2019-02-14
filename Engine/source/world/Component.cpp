#include "Component.h"

static long gComponentIndex = 0;

long ECSBase::next()
{
    return gComponentIndex++;
}