#ifndef APOCALYPSE_RANDOM_H

#include <stdlib.h>

inline float Rand01()
{
	return ((float) rand()) / ((float) RAND_MAX);
}

inline float RandFloat(float Min, float Max)
{
	return ((Max - Min) * Rand01()) + Min;
}

#define APOCALYPSE_RANDOM_H
#endif