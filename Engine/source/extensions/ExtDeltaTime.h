#pragma once
#include "V3.h"
#include "util/Time.h"

#define EXT_DELTA_TIME_LOAD() V3::getInstance()->addExtension(std::make_shared<ExtDeltaTime>(), 1000.0);

class ExtDeltaTime : public EngineExtension
{
public:
    double frameTimeStart = 0.0;
    unsigned int frameAccumulator = 0;

    void onTickBegin() override
    {
        frameTimeStart = Time::now();
    };

    void onTickEnd() override
    {
        V3::getInstance()->deltaTime = Time::now() - frameTimeStart;
        V3::getInstance()->elapsedTime = V3::getInstance()->elapsedTime + V3::getInstance()->deltaTime;
    };

    void tick() override
    {
        V3::getInstance()->framesPerSecond = frameAccumulator;
        //V3::getInstance()->getView()->setTitle(V3::getInstance()->debugWindowTitle + " - " + std::to_string(frameAccumulator) + " FPS, " + std::to_string(V3::getInstance()->deltaTime) + "ms");
        frameAccumulator = 0;
    };

    void tickWait() override
    {
        frameAccumulator += 1;
    };
};