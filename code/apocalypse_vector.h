#ifndef APOCALYPSE_VECTOR_H

#include <math.h>

struct vector2
{
	float X;
	float Y;
};

inline vector2 Vector2(float X, float Y)
{
	vector2 Result = {};
	Result.X = X;
	Result.Y = Y;
	return Result;
}

inline vector2 operator*(float A, vector2 B)
{
	vector2 Result;
	Result.X = A * B.X;
	Result.Y = A * B.Y;
	return Result;
}

inline vector2 operator*(vector2 B, float A)
{
	vector2 Result;
	Result.X = A * B.X;
	Result.Y = A * B.Y;
	return Result;	
}

inline vector2 operator*=(vector2 &B, float A)
{
	B = A * B;
	return B;
}

inline vector2 operator-(vector2 A)
{
	vector2 Result;
	Result.X = -1 * A.X;
	Result.Y = -1 * A.Y;
	return Result;
}

inline vector2 operator+(vector2 A, vector2 B)
{
	vector2 Result;
	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;
	return Result;
}

inline vector2 operator+=(vector2 &A, vector2 B)
{
	A = A + B;
	return A; 
}

inline vector2 operator-(vector2 A, vector2 B)
{
	vector2 Result;
	Result.X = A.X - B.X;
	Result.Y = A.Y - B.Y;
	return Result;
}

inline vector2 operator-=(vector2 &A, vector2 B)
{
	A = A - B;
	return A;
}

inline float Inner(vector2 A, vector2 B)
{
	return (A.X * B.X) + (A.Y * B.Y);
}

inline float MagnitudeSquared(vector2 A)
{
	return Inner(A, A);
}

inline float Magnitude(vector2 A)
{
	return (float) sqrt(MagnitudeSquared(A));
}

#define APOCALYPSE_VECTOR_H
#endif