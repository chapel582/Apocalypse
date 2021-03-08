#ifndef APOCALYPSE_RENDER_GROUP_H

#include "apocalypse_memory_arena.h"
#include "apocalypse_vector.h"
#include "apocalypse_bitmap.h"
#include "apocalypse_assets.h"

/* NOTE:
	1) Everywhere outside the renderer, Y _always_ goes upward, and X to the 
	right

	2) All bitmaps including the render target are assumed to be bottom up 
	(meaning that the first row pointer points to the bottom-most row when 
	viewed on screen)

	3) Unless otherwise specified, all inputs to the renderer are in world 
	units, NOT pixels. Anything that is in pixel values will be explicitly 
	marked as such.  
*/

struct basis
{
	// NOTE: if you use a basis for scaling all elements in a scene 
	// CONT: (e.g. camera space), the elements will also rotate
	// TODO: fix this? 

	// NOTE: the offset of the coordinate system from original space
	vector2 Offset;
	// NOTE: the axes as described in the original space's dimesions
	// NOTE: also for converting TO this basis
	vector2 Axis1;
	vector2 Axis2;
	// NOTE: for converting FROM this basis
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
	EntryType_Rectangle,
	EntryType_CoordinateSystem
} entry_type;

struct render_entry_header
{	
	entry_type Type;
	uint32_t Layer;
	uint32_t ClipRectIndex;
};

struct render_entry_clear
{
	render_entry_header Header;
	vector4 Color;
};

struct render_entry_bitmap
{
	render_entry_header Header;
	vector4 Color;
	
	vector2 Pos;
	vector2 XAxis;
	vector2 YAxis;

	loaded_bitmap* Bitmap;
	loaded_bitmap* NormalMap;
};

struct render_entry_rectangle
{
	render_entry_header Header;
	vector4 Color;
	vector2 Pos;
	vector2 XAxis;
	vector2 YAxis;
};

struct render_entry_handle
{
	render_entry_header* Header;
	uint32_t Layer;
};

struct render_group
{
	memory_arena* Arena;
	uint32_t NumEntries;
	
	// NOTE: camera basis is helpful so we can center the scaling of the world
	// CONT: somewhere besides the screen origin 
	basis* WorldToCamera;
	basis* CameraToScreen;

	rectangle ClipRects[16]; // TODO: make larger
	uint32_t NumClipRects;

	vector4 ColorMultiply;
};

typedef enum 
{
	PushText_Fail,
	PushText_Success	
} push_text_result_code;

struct push_text_result
{
	push_text_result_code Code;
	vector2 Offset; 
	// NOTE: a vector to the screen offset just past the last character
	// CONT: this could be used for something like the blinking cursor for text
	// CONT: editors
};

push_text_result PushText(
	render_group* Group,
	assets* Assets,
	font_handle FontHandle,
	uint32_t* CodePoints,
	uint32_t CodePointCount,
	float FontHeight, // NOTE: font height in world units
	vector2 LeftBaselinePoint,
	vector4 Color,
	uint32_t Layer = 1,
	uint32_t ClipRectIndex = 0
);
push_text_result PushText(
	render_group* Group,
	assets* Assets,
	font_handle FontHandle,
	char* CodePoints,
	uint32_t MaxCodePointCount,
	float FontHeight, // NOTE: font height in world units
	vector2 LeftBaselinePoint,
	vector4 Color,
	memory_arena* FrameArena, // NOTE: this function will leak if you don't
	// CONT: regularly clear the arena. Hence, FrameArena
	uint32_t Layer = 1,
	uint32_t ClipRectIndex = 0
);
push_text_result PushTextCentered(
	render_group* Group,
	assets* Assets,
	font_handle FontHandle,
	char* CodePoints,
	uint32_t MaxCodePointCount,
	float FontHeight, 
	vector2 Center,
	vector4 Color,
	memory_arena* FrameArena,
	uint32_t Layer = 1,
	uint32_t ClipRectIndex = 0
);
void PushClear(render_group* Group, vector4 Color, uint32_t Layer = 0);
vector2 TransformPosFromBasis(basis* Basis, vector2 Vector);
vector2 TransformVectorToBasis(basis* Basis, vector2 Vector);
uint32_t AddClipRect(render_group* RenderGroup, rectangle ToAdd);

#define APOCALYPSE_RENDER_GROUP_H
#endif