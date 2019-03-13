#pragma once
#include "Tickable.h"
#include <memory>

class V3;

class Module : public Tickable
{
public:
	V3* v3;
	unsigned int type_id;
    double interval = 0.0;
    double intervalAccumulator = 0.0;

    virtual void onStartup() { };

    virtual void onTickBegin() { };

    virtual void onTickEnd() { };

    virtual void onTick() { };

    virtual void onPreRender() { };

    virtual void onPostRender() { };

	virtual void onShutdown() { };

    virtual void tickWait() { };
};