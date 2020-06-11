#ifndef APOCALYPSE_MATH_H

#include "apocalypse_platform.h"

inline float Lerp(float A, float T, float B)
{
    float Result = (1.0f - T) * A + T * B;

    return Result;
}

#define APOCALYPSE_MATH_H
#endif