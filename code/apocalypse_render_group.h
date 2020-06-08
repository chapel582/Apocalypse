#ifndef APOCALYPSE_RENDER_GROUP_H

#include "apocalypse_memory_arena.h"
#include "apocalypse_vector.h"
#include "apocalypse_bitmap.h"

// TODO: get rid of world_screen_converter and just use basis
// TODO: hopefully that will also work with mouse clicks
// struct world_screen_converter
// {
// 	float ScreenToWorld;
// 	float WorldToScreen;
// 	// NOTE: ScreenYOffset is where the screen origin is relative to the 
// 	// CONT: World origin but in pixels
// 	float ScreenYOffset; 
// 	// NOTE: WorldYOffset is where the world origin is relative to the Screen 
// 	// CONT: origin but in world units
// 	float WorldYOffset;
// };

struct basis
{
	// NOTE: the offset of the coordinate system from world's origin
	vector2 Offset;
	// NOTE: the axes as described in world space
	// NOTE: also for converting FROM this basis
	vector2 Axis1;
	vector2 Axis2;
	// NOTE: for converting TO this basis
	vector2 MatrixColumn1;
	vector2 MatrixColumn2;
};

inline basis MakeBasis(vector2 Offset, vector2 Axis1, vector2 Axis2)
{
	basis Result = {};
	Result.Offset = Offset;
	Result.Axis1 = Axis1;
	Result.Axis2 = Axis2;
	float Determinant = (
		(Result.Axis1.X * Result.Axis2.Y) - (Result.Axis2.X * Result.Axis1.Y)
	);
	float OneOverDeterminant = 1.0f / Determinant;
	Result.MatrixColumn1 = (
		OneOverDeterminant * 
		Vector2(Result.Axis2.Y, -1 * Result.Axis1.Y)
	);
	Result.MatrixColumn2 = (
		OneOverDeterminant * 
		Vector2(-1 * Result.Axis2.X, Result.Axis1.X)
	);
	return Result;
}

typedef enum 
{
	EntryType_Clear,
	EntryType_Bitmap,
	EntryType_Rectangle	
} entry_type_e;

struct render_entry_header
{	
	entry_type_e Type;
};

struct render_entry_clear
{
	render_entry_header Header;
	vector4 Color;
};

struct render_entry_bitmap
{
	render_entry_header Header;
	basis Basis; // NOTE: basis in world space
	loaded_bitmap* Bitmap;
	vector2 Position; // NOTE: position in basis
};

struct render_entry_rectangle
{
	render_entry_header Header;
	basis Basis; // NOTE: basis in world space
	vector4 Color;
	vector2 Dim;
	vector2 Position; // NOTE: position in basis
};

struct render_group
{
	memory_arena* Arena;
	uint8_t* LastEntry;
	
	basis DefaultBasis;
	basis* WorldScreenBasis;
	// NOTE: CameraPos is where the bottom left of camera rectangle is in the
	// CONT: world
	vector2 CameraPos;
};

#define APOCALYPSE_RENDER_GROUP_H
#endif