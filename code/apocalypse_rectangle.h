#ifndef APOCALYPSE_RECTANGLE_H

#include "apocalypse_vector.h"

struct rectangle
{
	vector2 Min; // NOTE: Bottom left by convention
	vector2 Dim;
};

inline bool Equivalent(rectangle A, rectangle B)
{
	return (A.Min == B.Min) && (A.Dim == B.Dim);
}

inline rectangle MakeRectangle(vector2 Min, vector2 Dim)
{
	rectangle Result;
	Result.Min = Min;
	Result.Dim = Dim;
	return Result;
}

inline rectangle MakeRectangleCentered(vector2 Center, vector2 Dim)
{
	rectangle Result;
	Result.Min = Center - 0.5f * Dim;
	Result.Dim = Dim;
	return Result;
}

inline float GetRight(rectangle Rectangle)
{
	return Rectangle.Min.X + Rectangle.Dim.X;
}

inline float GetTop(rectangle Rectangle)
{
	return Rectangle.Min.Y + Rectangle.Dim.Y;
}

inline vector2 GetTopLeft(rectangle Rectangle)
{
	vector2 Result;
	Result.X = Rectangle.Min.X;
	Result.Y = GetTop(Rectangle);
	return Result;
}

inline vector2 GetTopRight(rectangle Rectangle)
{
	vector2 Result;
	Result.X = GetRight(Rectangle);
	Result.Y = GetTop(Rectangle);
	return Result;
}

inline float GetBottom(rectangle Rectangle)
{
	return Rectangle.Min.Y;
}

inline vector2 GetBottomLeft(rectangle Rectangle)
{
	return Rectangle.Min;
}

inline vector2 GetCenter(rectangle Rectangle)
{
	return Rectangle.Min + (0.5f * Rectangle.Dim);
}

inline void SetLeft(rectangle* Rectangle, float Left)
{
	Rectangle->Min.X = Left;
}

inline void SetBottom(rectangle* Rectangle, float Bottom)
{
	Rectangle->Min.Y = Bottom;
}

inline void SetTop(rectangle* Rectangle, float Top)
{
	Rectangle->Min.Y = Top - Rectangle->Dim.Y;  
}

inline void SetTopLeft(rectangle* Rectangle, vector2 TopLeft)
{
	Rectangle->Min.X = TopLeft.X;
	SetTop(Rectangle, TopLeft.Y);
}

inline void SetCenter(rectangle* Rectangle, vector2 Center)
{
	SetLeft(Rectangle, Center.X - (0.5f * Rectangle->Dim.X));
	SetTop(Rectangle, Center.Y - (0.5f * Rectangle->Dim.Y));
}

bool PointInRectangle(vector2 Point, rectangle Rectangle)
{
	vector2 Max = Rectangle.Min + Rectangle.Dim; 
	return (
		(Point.X >= Rectangle.Min.X && Point.X < Max.X) && 
		(Point.Y >= Rectangle.Min.Y && Point.Y < Max.Y)
	);
}

inline void ClampRectY(rectangle* Rectangle, float MinY, float MaxY)
{
	if(GetBottom(*Rectangle) < MinY)
	{
		SetBottom(Rectangle, MinY);
	}
	if(GetTop(*Rectangle) > MaxY)
	{
		SetTop(Rectangle, MaxY);				
	}
}

rectangle MakeNextRectangle(
	float XPos, float YStart, float YMargin, vector2 Dim, uint32_t Index
)
{
	// NOTE: Makes the next vertical rectangle in a vertical stack of 
	// CONT: rectangles
	return MakeRectangle(
		Vector2(XPos, YStart - (Dim.Y + YMargin) * (Index + 1)), Dim
	);
}

#define APOCALYPSE_RECTANGLE_H
#endif