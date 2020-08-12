#ifndef APOCALYPSE_RECTANGLE_H

struct rectangle
{
	vector2 Min; // NOTE: Bottom left by convention
	vector2 Dim;
};

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

inline vector2 GetTopLeft(rectangle Rectangle)
{
	vector2 Result;
	Result.X = Rectangle.Min.X;
	Result.Y = Rectangle.Min.Y + Rectangle.Dim.Y;
	return Result;
}

inline vector2 GetBottomLeft(rectangle Rectangle)
{
	return Rectangle.Min;
}

inline vector2 GetCenter(rectangle Rectangle)
{
	return Rectangle.Min + (0.5f * Rectangle.Dim);
}

bool PointInRectangle(vector2 Point, rectangle Rectangle)
{
	vector2 Max = Rectangle.Min + Rectangle.Dim; 
	return (
		(Point.X >= Rectangle.Min.X && Point.X < Max.X) && 
		(Point.Y >= Rectangle.Min.Y && Point.Y < Max.Y)
	);
}

#define APOCALYPSE_RECTANGLE_H
#endif