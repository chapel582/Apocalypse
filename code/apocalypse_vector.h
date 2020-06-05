#ifndef APOCALYPSE_VECTOR_H

#include <math.h>

// SECTION START: vector2
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
// SECTION STOP: vector2

// SECTION START: vector3
struct vector3
{
	float X;
	float Y;
	float Z;
};

inline vector3 Vector3(float X, float Y, float Z)
{
	vector3 Result = {};
	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;
	return Result;
}

inline vector3 Vector3(vector3 V, float Z)
{
	vector3 Result = {};
	Result.X = V.X;
	Result.Y = V.Y;
	Result.Z = Z;
	return Result;
}

inline vector3 operator*(float A, vector3 B)
{
	vector3 Result;
	Result.X = A * B.X;
	Result.Y = A * B.Y;
	Result.Z = A * B.Z;
	return Result;
}

inline vector3 operator*(vector3 B, float A)
{
	vector3 Result;
	Result.X = A * B.X;
	Result.Y = A * B.Y;
	Result.Z = A * B.Z;
	return Result;
}

inline vector3 operator*=(vector3 &B, float A)
{
	B = A * B;
	return B;
}

inline vector3 operator-(vector3 A)
{
	vector3 Result;
	Result.X = -1 * A.X;
	Result.Y = -1 * A.Y;
	Result.Z = -1 * A.Z;
	return Result;
}

inline vector3 operator+(vector3 A, vector3 B)
{
	vector3 Result;
	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;
	Result.Z = A.Z + B.Z;
	return Result;
}

inline vector3 operator+=(vector3 &A, vector3 B)
{
	A = A + B;
	return A; 
}

inline vector3 operator-(vector3 A, vector3 B)
{
	vector3 Result;
	Result.X = A.X - B.X;
	Result.Y = A.Y - B.Y;
	Result.Z = A.Z - B.Z;
	return Result;
}

inline vector3 operator-=(vector3 &A, vector3 B)
{
	A = A - B;
	return A;
}

inline float Inner(vector3 A, vector3 B)
{
	return (A.X * B.X) + (A.Y * B.Y) + (A.Z * B.Z);
}

inline float MagnitudeSquared(vector3 A)
{
	return Inner(A, A);
}

inline float Magnitude(vector3 A)
{
	return (float) sqrt(MagnitudeSquared(A));
}
// SECTION STOP: vector3

// SECTION START: vector4
union vector4
{
	struct
	{
		float X;
		float Y;
		float Z;
		float W;
	};
	struct
	{
		float R;
		float G;
		float B;
		float A;
	};
};

inline vector4 Vector4(float X, float Y, float Z, float W)
{
	vector4 Result = {};
	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;
	Result.W = W;
	return Result;
}
// SECTION STOP vector4

#define APOCALYPSE_VECTOR_H
#endif