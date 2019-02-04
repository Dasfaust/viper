#pragma once
#include "Tickable.h"

class EngineExtension : public Tickable
{
public:
    double interval = 0.0;
    double intervalAccumulator = 0.0;

    virtual void onTickBegin() { };

    virtual void onTickEnd() { };

    virtual void onPreRender() { };

    virtual void onPostRender() { };

    virtual void tickWait() { };
};