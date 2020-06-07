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
	render_basis* Basis;
	loaded_bitmap* Bitmap;
	vector2 Position;
};

struct render_entry_rectangle
{
	render_entry_header Header;
	render_basis* Basis;
	vector4 Color;
	vector2 Dim;
	vector2 Position;
};

struct render_group
{
	memory_arena* Arena;
	uint8_t* LastEntry;
	
	render_basis DefaultBasis;
	world_screen_converter WorldScreenConverter;
	// NOTE: CameraPos is where the bottom left of camera rectangle is in the
	// CONT: world
	vector2 CameraPos;
};

#define APOCALYPSE_RENDER_GROUP_H
#endif