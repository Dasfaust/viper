#include "DeltaTime.h"
#include "../V3.h"

void DeltaTime::onTickBegin()
{
	frameTimeStart = Time::now();
};

void DeltaTime::onTickEnd()
{
	deltaTime = Time::now() - frameTimeStart;
	elapsedTime = elapsedTime + deltaTime;
};

void DeltaTime::onTick()
{
	framesPerSecond = frameAccumulator;
	frameAccumulator = 0;
};

void DeltaTime::tickWait()
{
	frameAccumulator += 1;
};