#ifndef APOCALYPSE_RENDER_GROUP_H

#include "apocalypse_memory_arena.h"
#include "apocalypse_vector.h"
#include "apocalypse_bitmap.h"

// TODO: get rid of world_screen_converter and just use basis
// TODO: hopefully that will also work with mouse clicks
struct world_screen_converter
{
	float ScreenToWorld;
	float WorldToScreen;
	// NOTE: ScreenYOffset is where the screen origin is relative to the 
	// CONT: World origin but in pixels
	float ScreenYOffset; 
	// NOTE: WorldYOffset is where the world origin is relative to the Screen 
	// CONT: origin but in world units
	float WorldYOffset;
};

struct render_basis
{
	vector3 P;
};

typedef enum 
{
	PieceType_Bitmap,
	PieceType_Rectangle	
} piece_type_e;

struct visible_piece
{
	render_basis* Basis;
	loaded_bitmap* Bitmap;

	piece_type_e Type;

	vector4 Color;
	vector2 Position;
	vector2 Dim;
};

struct render_group
{
	memory_arena* Arena;
	visible_piece* LastPiece;
	
	render_basis DefaultBasis;
	world_screen_converter WorldScreenConverter;
	// NOTE: CameraPos is where the bottom left of camera rectangle is in the
	// CONT: world
	vector2 CameraPos;
};

#define APOCALYPSE_RENDER_GROUP_H
#endif