#pragma once
#include "Component.h"
#include <memory>
#include <vector>

struct Entity
{
public:
    std::vector<std::shared_ptr<ComponentBase>> components;
};