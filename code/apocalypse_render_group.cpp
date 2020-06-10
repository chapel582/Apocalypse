#include "apocalypse_render_group.h"

#include "apocalypse_rectangle.h"
#include "apocalypse_vector.h"

vector2 WorldToBasis(basis* Basis, vector2 Vector)
{
	vector2 Result = {};
	vector2 Position = Vector - Basis->Offset;
	Result = (
		Position.X * Basis->MatrixColumn1 + 
		Position.Y * Basis->MatrixColumn2 
	);
	return Result;
}

vector2 BasisToWorld(basis* Basis, vector2 Vector)
{
	vector2 Result = {};
	Result = (
		Vector.X * Basis->Axis1 + 
		Vector.Y * Basis->Axis2
	);
	Result += Basis->Offset;
	return Result;
}

vector2 WorldToBasisScale(basis* Basis, vector2 Vector)
{
	vector2 Result = {};
	Result = (
		Vector.X * Basis->MatrixColumn1 + 
		Vector.Y * Basis->MatrixColumn2
	);
	Result = Abs(Result);
	return Result;
}

vector2 BasisToWorldScale(basis* Basis, vector2 Vector)
{
	vector2 Result = {};
	Result = (
		Vector.X * Basis->Axis1 + 
		Vector.Y * Basis->Axis2
	);
	return Result;
}

vector2 ScreenToWorldPos(basis* WorldScreenBasis, uint32_t X, uint32_t Y)
{
	return BasisToWorld(WorldScreenBasis, Vector2(X, Y));
}

vector2 WorldToScreenPos(basis* WorldScreenBasis, vector2 WorldPosition)
{
	return WorldToBasis(WorldScreenBasis, WorldPosition);
}

vector2 ScreenToWorldDim(basis* WorldScreenBasis, vector2 ScreenDim)
{
	return BasisToWorldScale(WorldScreenBasis, ScreenDim);
}

vector2 WorldToScreenDim(basis* WorldScreenBasis, vector2 WorldDim)
{
	return WorldToBasisScale(WorldScreenBasis, WorldDim);
}

inline void PushBitmap(
	render_group* Group,
	basis Basis,
	loaded_bitmap* Bitmap,
	vector2 WorldTopLeft
)
{
	render_entry_bitmap* Entry = PushStruct(Group->Arena, render_entry_bitmap);
	Group->LastEntry = (uint8_t*) Entry;
	Entry->Header.Type = EntryType_Bitmap;
	Entry->Basis = Basis;
	Entry->Bitmap = Bitmap;
	Entry->Position = WorldTopLeft;
}

// TODO: make a push rect that doesn't require pushing a basis and will make it based on the rect you push
inline void PushRect(
	render_group* Group, basis Basis, rectangle Rectangle, vector4 Color
)
{
	render_entry_rectangle* Entry = PushStruct(
		Group->Arena, render_entry_rectangle
	);
	Group->LastEntry = (uint8_t*) Entry;
	Entry->Header.Type = EntryType_Rectangle;
	Entry->Basis = Basis;
	Entry->Color = Color;
	Entry->Dim = Rectangle.Dim;
}

inline void PushClear(render_group* Group, vector4 Color)
{
	render_entry_clear* Entry = PushStruct(Group->Arena, render_entry_clear);
	Group->LastEntry = (uint8_t*) Entry;
	Entry->Header.Type = EntryType_Clear;
	Entry->Color = Color;
}

inline void PushCoordinateSystem(render_group* Group, basis Basis)
{
	render_entry_coordinate_system* Entry = PushStruct(
		Group->Arena, render_entry_coordinate_system
	);
	Group->LastEntry = (uint8_t*) Entry;
	Entry->Header.Type = EntryType_CoordinateSystem;
	Entry->Basis = Basis;
}

void DrawRectangle(
	game_offscreen_buffer* BackBuffer, 
	rectangle Rectangle,
	float Red, 
	float Green, 
	float Blue
)
{
	int32_t MinX = RoundFloat32ToInt32(Rectangle.Min.X);
	int32_t MinY = RoundFloat32ToInt32(Rectangle.Min.Y);
	int32_t MaxX = RoundFloat32ToInt32(Rectangle.Min.X + Rectangle.Dim.X);
	int32_t MaxY = RoundFloat32ToInt32(Rectangle.Min.Y + Rectangle.Dim.Y);

	if(MinX < 0)
	{
		MinX = 0;
	}
	else if(MinX > BackBuffer->Width)
	{
		MinX = BackBuffer->Width;
	}
	if(MaxX < 0)
	{
		MaxX = 0;
	}
	else if(MaxX > BackBuffer->Width)
	{
		MaxX = BackBuffer->Width;
	}
	if(MinY < 0)
	{
		MinY = 0;
	}
	else if(MinY > BackBuffer->Height)
	{
		MinY = BackBuffer->Height;
	}
	if(MaxY < 0)
	{
		MaxY = 0;
	}
	else if(MaxY > BackBuffer->Height)
	{
		MaxY = BackBuffer->Height;
	}

	uint32_t Color = (
		(RoundFloat32ToInt32(Red * 0xFF) << 16) |
		(RoundFloat32ToInt32(Green * 0xFF) << 8) |
		RoundFloat32ToInt32(Blue * 0xFF)
	);

	uint8_t* Row = (
		((uint8_t*) BackBuffer->Memory) + 
		MinX * BackBuffer->BytesPerPixel +
		MinY * BackBuffer->Pitch 
	);

	for(int Y = MinY; Y < MaxY; Y++)
	{
		uint32_t* Pixel = (uint32_t*) Row; 
		for(int X = MinX; X < MaxX; X++)
		{
			*Pixel++ = Color;
		}
		Row += BackBuffer->Pitch;
	}
}

void DrawRectangleSlowly(
	game_offscreen_buffer* BackBuffer, 
	vector2 Origin,
	vector2 XAxis,
	vector2 YAxis,
	vector4 ColorV
)
{
	float fMinX = (float) BackBuffer->Width;
	float fMinY = (float) BackBuffer->Height;
	float fMaxX = 0;
	float fMaxY = 0;

	vector2 Points[4] = {
		Origin,
		Origin + XAxis,
		Origin + YAxis,
		Origin + XAxis + YAxis
	};
	for(int Index = 0; Index < ARRAY_COUNT(Points); Index++)
	{
		vector2 Point = Points[Index];
		if(Point.X < fMinX)
		{
			fMinX = Point.X;
		}
		if(Point.X > fMaxX)
		{
			fMaxX = Point.X;
		}

		if(Point.Y < fMinY)
		{
			fMinY = Point.Y;
		}
		if(Point.Y > fMaxY)
		{
			fMaxY = Point.Y;
		}
	}

	int32_t MinX = RoundFloat32ToInt32(fMinX);
	int32_t MaxX = RoundFloat32ToInt32(fMaxX);
	int32_t MinY = RoundFloat32ToInt32(fMinY);
	int32_t MaxY = RoundFloat32ToInt32(fMaxY);

	if(MinX < 0)
	{
		MinX = 0;
	}
	else if(MinX > BackBuffer->Width)
	{
		MinX = BackBuffer->Width;
	}
	if(MaxX < 0)
	{
		MaxX = 0;
	}
	else if(MaxX > BackBuffer->Width)
	{
		MaxX = BackBuffer->Width;
	}
	if(MinY < 0)
	{
		MinY = 0;
	}
	else if(MinY > BackBuffer->Height)
	{
		MinY = BackBuffer->Height;
	}
	if(MaxY < 0)
	{
		MaxY = 0;
	}
	else if(MaxY > BackBuffer->Height)
	{
		MaxY = BackBuffer->Height;
	}

	uint32_t Color = (
		(RoundFloat32ToInt32(ColorV.R * 0xFF) << 16) |
		(RoundFloat32ToInt32(ColorV.G * 0xFF) << 8) |
		RoundFloat32ToInt32(ColorV.B * 0xFF)
	);

	vector2 XAxisPerp = Perpendicular(XAxis);
	vector2 YAxisPerp = Perpendicular(YAxis);

	uint8_t* Row = (
		((uint8_t*) BackBuffer->Memory) + 
		MinX * BackBuffer->BytesPerPixel +
		MinY * BackBuffer->Pitch 
	);
	for(int Y = MinY; Y < MaxY; Y++)
	{
		uint32_t* Pixel = (uint32_t*) Row; 
		for(int X = MinX; X < MaxX; X++)
		{
			vector2 Point = Vector2(X, Y);
			float Edge0 = Inner(Point - Origin, XAxisPerp);
			float Edge1 = Inner(Point - (Origin + XAxis), YAxisPerp);
			float Edge2 = Inner(
				Point - (Origin + XAxis + YAxis), -1 * XAxisPerp
			);
			float Edge3 = Inner(Point - (Origin + YAxis), -1 * YAxisPerp);

			if((Edge0 < 0) && (Edge1 < 0) && (Edge2 < 0) && (Edge3 < 0))
			{
				*Pixel = Color;
			}
			Pixel++;
		}
		Row += BackBuffer->Pitch;
	}
}

void DrawBitmap(
	game_offscreen_buffer* Buffer,
	loaded_bitmap* Bitmap,
	float RealX,
	float RealY,
	int32_t AlignX = 0,
	int32_t AlignY = 0
)
{
	RealX -= (float) AlignX;
	RealY -= (float) AlignY;

	int32_t MinX = RoundFloat32ToInt32(RealX);
	int32_t	MinY = RoundFloat32ToInt32(RealY);
	int32_t MaxX = RoundFloat32ToInt32(RealX + (float) Bitmap->Width);
	int32_t MaxY = RoundFloat32ToInt32(RealY + (float) Bitmap->Height);

	// NOTE: X and Y Offsets are so we start drawing from the correct place if 
	// CONT: part of the BMP is offscreen
	int32_t SourceOffsetX = 0;
	if(MinX < 0)
	{
		SourceOffsetX = -MinX;
		MinX = 0;
	}

	int32_t SourceOffsetY = 0;
	if(MinY < 0)
	{
		SourceOffsetY = -MinY;
		MinY = 0;
	}

	if(MaxX > Buffer->Width)
	{
		MaxX = Buffer->Width;
	}

	if(MaxY > Buffer->Height)
	{
		MaxY = Buffer->Height;
	}

	// TODO: SourceRow needs to change based on clipping
	// NOTE: bitmaps are stored upside down
	// CONT: Bitmap->Width * (Bitmap->Height - 1) means we start at the last row
	uint32_t* SourceRow = Bitmap->Pixels + Bitmap->Width * (Bitmap->Height - 1);
	SourceRow += -SourceOffsetY * Bitmap->Width + SourceOffsetX;
	uint8_t* DestRow = (
		(uint8_t*) Buffer->Memory +
		MinX * Buffer->BytesPerPixel + 
		MinY * Buffer->Pitch
	);

	for(int Y = MinY; Y < MaxY; Y++)
	{
		uint32_t* Dest = (uint32_t*) DestRow;
		uint32_t* Source = SourceRow;
		for(int X = MinX; X < MaxX; X++)
		{
			// TODO: premultiplied alpha
			float A = ((float) ((*Source >> 24) & 0xFF)) / 255.0f;
			float SR = (float) (((*Source >> 16) & 0xFF));
			float SG = (float) (((*Source >> 8) & 0xFF));
			float SB = (float) (((*Source) & 0xFF));

			float DR = (float) (((*Dest >> 16) & 0xFF));
			float DG = (float) (((*Dest >> 8) & 0xFF));
			float DB = (float) (((*Dest) & 0xFF));

			float R = ((1.0f - A) * DR) + (A * SR);
			float G = ((1.0f - A) * DG) + (A * SG);
			float B = ((1.0f - A) * DB) + (A * SB);

			*Dest = (
				(((uint32_t) (R + 0.5f)) << 16) |
				(((uint32_t) (G + 0.5f)) << 8) |
				((uint32_t) (B + 0.5f))
			);

			Dest++;
			Source++;
		}
		DestRow += Buffer->Pitch;
		SourceRow -= Bitmap->Width;
	}
}

void Clear(
	game_offscreen_buffer* BackBuffer, 
	float Red,
	float Green, 
	float Blue
)
{
	DrawRectangle(
		BackBuffer,
		MakeRectangle(
			Vector2(0.0f, 0.0f),
			Vector2((float) BackBuffer->Width, (float) BackBuffer->Height)
		),
		Red,
		Green,
		Blue
	);
}

vector2 GetScreenPos(
	basis* Basis,
	basis* WorldScreenBasis,
	vector2 EntryPosition,
	vector2 CameraPos
)
{
	vector2 WorldPos = BasisToWorld(Basis, EntryPosition);
	vector2 CameraTopLeft = WorldPos - CameraPos;

	return WorldToBasis(WorldScreenBasis, CameraTopLeft);
}

void RenderGroupToOutput(
	render_group* RenderGroup, game_offscreen_buffer* BackBuffer
)
{
	for(
		uint8_t* CurrentAddress = RenderGroup->Arena->Base;
		CurrentAddress <= (RenderGroup->LastEntry);
	)
	{
		render_entry_header* Header = (render_entry_header*) CurrentAddress; 
		switch(Header->Type)
		{
			case(EntryType_Clear):
			{
				render_entry_clear* Entry = (render_entry_clear*) Header;
				Clear(
					BackBuffer, Entry->Color.R, Entry->Color.G, Entry->Color.B
				);
				CurrentAddress += sizeof(*Entry);
				break;
			}
			case(EntryType_Rectangle):
			{
				render_entry_rectangle* Entry = (render_entry_rectangle*) (
					Header
				);
				vector2 OriginScreenPos = GetScreenPos(
					&Entry->Basis,
					RenderGroup->WorldScreenBasis,
					Vector2(0, 0),
					RenderGroup->CameraPos
				);

				vector2 Axis1ScreenPos = GetScreenPos(
					&Entry->Basis,
					RenderGroup->WorldScreenBasis,
					Entry->Dim.X * Entry->Basis.Axis1,
					RenderGroup->CameraPos
				);
				vector2 Axis2ScreenPos = GetScreenPos(
					&Entry->Basis,
					RenderGroup->WorldScreenBasis,
					Entry->Dim.Y * Entry->Basis.Axis2,
					RenderGroup->CameraPos
				);

				vector2 Axis1 = Axis1ScreenPos - OriginScreenPos;
				// Axis1.Y = -1 * Axis1.Y;
				vector2 Axis2 = Axis2ScreenPos - OriginScreenPos;
				// Axis2.Y = -1 * Axis2.Y;

				DrawRectangleSlowly(
					BackBuffer,
					OriginScreenPos,
					Axis1,
					Axis2,
					Entry->Color
				);

				vector2 RectangleDim = Vector2(10, 10);

				DrawRectangle(
					BackBuffer,
					MakeRectangle(OriginScreenPos, RectangleDim),
					0.0f,
					0.0f,
					1.0f
				);
				DrawRectangle(
					BackBuffer,
					MakeRectangle(Axis1ScreenPos, RectangleDim),
					0.0f,
					0.0f,
					1.0f
				);
				DrawRectangle(
					BackBuffer,
					MakeRectangle(Axis2ScreenPos, RectangleDim),
					0.0f,
					0.0f,
					1.0f
				);
				CurrentAddress += sizeof(*Entry);
				break;
			}
			case(EntryType_Bitmap):
			{
				render_entry_bitmap* Entry = (render_entry_bitmap*) Header;

				vector2 ScreenPos = GetScreenPos(
					&Entry->Basis,
					RenderGroup->WorldScreenBasis,
					Entry->Position,
					RenderGroup->CameraPos
				);

				DrawBitmap(
					BackBuffer,
					Entry->Bitmap,
					ScreenPos.X,
					ScreenPos.Y
				);
				CurrentAddress += sizeof(*Entry);
				break;
			}
			default:
			{
				ASSERT(false);
			}
		}
	}
	ResetMemArena(RenderGroup->Arena);
}
