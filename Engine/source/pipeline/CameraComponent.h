#pragma once
#include "../world/ECS.h"

class CameraComponent : public Component<CameraComponent>
{
public:
};

class CameraUpdater : public ComponentTicker<CameraComponent>
{
public:
};