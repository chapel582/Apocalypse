#ifndef APOCALYPSE_MATH_H

#include "apocalypse_platform.h"

inline float Lerp(float A, float T, float B)
{
	float Result = (1.0f - T) * A + T * B;

	return Result;
}

inline float Clamp(float Min, float Value, float Max)
{
	float Result = Value;

	if(Result < Min)
	{
		Result = Min;
	}
	else if(Result > Max)
	{
		Result = Max;
	}

	return Result;
}

inline float Clamp01(float Value)
{
	float Result = Clamp(0.0f, Value, 1.0f);

	return Result;
}

inline float SafeRatioN(float Dividend, float Divisor, float N)
{
	float Result;
	if(Divisor != 0.0f)
	{
		Result = Dividend / Divisor;
	}
	else
	{
		Result = N;
	}
	return Result;
}

inline float SafeRatio0(float Dividend, float Divisor)
{
	return SafeRatioN(Dividend, Divisor, 0.0f);
}

inline float SafeRatio1(float Dividend, float Divisor)
{
    return SafeRatioN(Dividend, Divisor, 1.0f);
}

#define APOCALYPSE_MATH_H
#endif