#pragma once
#include "../math/Math.h"
#include <memory>
#include <vector>

class ECSBase
{

};

template<class Handler>
class Component : ECSBase
{
public:
    Component<T> currentStep;
    Component<T> lastStep;
    float alpha = 0.0f;

    virtual inline void init()
    {

    };
    
    virtual inline void tick()
    {

    };

    virtual inline void tickWait()
    {

    };

    virtual inline void removed()
    {

    };
};

class Entity
{
public:
    std::vector<std::shared_ptr<ECSBase>> components;
};