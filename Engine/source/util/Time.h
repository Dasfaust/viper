#pragma once
#include <chrono>

#define tnow() Time::now()

namespace Time
{
    static double now()
    {
        return (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }
}