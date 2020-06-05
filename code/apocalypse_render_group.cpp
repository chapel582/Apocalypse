#include "apocalypse_render_group.h"

#include "apocalypse_vector.h"

vector2 ScreenToWorldPos(
	world_screen_converter Converter, uint32_t X, uint32_t Y
)
{
	vector2 Result = {};
	Result.X = Converter.ScreenToWorld * X;
	Result.Y = (
		(-1.0f * Converter.ScreenToWorld * Y) + Converter.WorldYOffset
	);
	return Result;
}

vector2 WorldToScreenPos(
	world_screen_converter Converter, vector2 WorldPosition
)
{
	vector2 Result = {};
	Result.X = Converter.WorldToScreen * WorldPosition.X;
	Result.Y = (
		(-1.0f * Converter.WorldToScreen * WorldPosition.Y) + 
		Converter.ScreenYOffset
	);
	return Result;
}

vector2 ScreenToWorldDim(world_screen_converter Converter, vector2 ScreenDim)
{
	vector2 Result = {};
	Result.X = Converter.ScreenToWorld * ScreenDim.X;
	Result.Y = Converter.ScreenToWorld * ScreenDim.Y;
	return Result;
}

vector2 WorldToScreenDim(world_screen_converter Converter, vector2 WorldDim)
{
	vector2 Result = {};
	Result.X = Converter.WorldToScreen * WorldDim.X;
	Result.Y = Converter.WorldToScreen * WorldDim.Y;
	return Result;
}

inline void PushCommon(
	render_group* Group,
	render_basis* Basis,
	loaded_bitmap* Bitmap,
	piece_type_e Type,
	vector2 Position,
	vector4 Color,
	vector2 Dim
)
{
	visible_piece* Piece = PushStruct(Group->Arena, visible_piece);
	Piece->Basis = Basis;
	Piece->Bitmap = Bitmap;
	Piece->Type = Type;
	Piece->Position = Position;
	Piece->Color = Color;
	Piece->Dim = Dim;
	Group->LastPiece = Piece;
}

inline void PushBitmap(
	render_group* Group, loaded_bitmap* Bitmap, vector2 WorldTopLeft
)
{
	vector2 CameraTopLeft = WorldTopLeft - Group->CameraPos;
	vector2 ScreenPos = WorldToScreenPos(
		Group->WorldScreenConverter, CameraTopLeft
	);
	vector2 ScreenDim = WorldToScreenDim(
		Group->WorldScreenConverter,
		Vector2((float) Bitmap->Width, (float) Bitmap->Height)
	);

	PushCommon(
		Group,
		&Group->DefaultBasis,
		NULL,
		PieceType_Bitmap,
		ScreenPos,
		Vector4(0, 0, 0, 0),
		ScreenDim
	);
}

inline void PushRect(render_group* Group, rectangle Rectangle, vector4 Color)
{
	// NOTE: get rectangle's world position relative to camera
	// CONT: then convert position relative to camera to screen space 
	vector2 WorldTopLeft = GetTopLeft(Rectangle);
	vector2 CameraTopLeft = WorldTopLeft - Group->CameraPos;
	vector2 ScreenPos = WorldToScreenPos(
		Group->WorldScreenConverter, CameraTopLeft
	);
	vector2 ScreenDim = WorldToScreenDim(
		Group->WorldScreenConverter, Rectangle.Dim
	);

	PushCommon(
		Group,
		&Group->DefaultBasis,
		NULL,
		PieceType_Rectangle,
		ScreenPos,
		Color,
		ScreenDim
	);
}