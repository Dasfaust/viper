#pragma once
#include "../Module.h"
#include "../util/Time.h"

class DeltaTime : public Module
{
public:
	unsigned int framesPerSecond = 0;

    double frameTimeStart = 0.0;
    unsigned int frameAccumulator = 0;

	double deltaTime = 0.0;
	double elapsedTime = 0.0;

	void onTickBegin() override;

	void onTickEnd() override;

	void onTick() override;

	void tickWait() override;
};