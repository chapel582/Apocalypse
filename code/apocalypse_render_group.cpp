#include "apocalypse_render_group.h"

#include "apocalypse_rectangle.h"
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

inline void PushBitmap(
	render_group* Group, loaded_bitmap* Bitmap, vector2 WorldTopLeft
)
{
	vector2 CameraTopLeft = WorldTopLeft - Group->CameraPos;
	vector2 ScreenPos = WorldToScreenPos(
		Group->WorldScreenConverter, CameraTopLeft
	);

	render_entry_bitmap* Entry = PushStruct(Group->Arena, render_entry_bitmap);
	Group->LastEntry = (uint8_t*) Entry;
	Entry->Header.Type = EntryType_Bitmap;
	Entry->Basis = &Group->DefaultBasis;
	Entry->Bitmap = Bitmap;
	Entry->Position = ScreenPos;
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

	render_entry_rectangle* Entry = PushStruct(
		Group->Arena, render_entry_rectangle
	);
	Group->LastEntry = (uint8_t*) Entry;
	Entry->Header.Type = EntryType_Rectangle;
	Entry->Basis = &Group->DefaultBasis;
	Entry->Color = Color;
	Entry->Dim = ScreenDim;
	Entry->Position = ScreenPos;
}

inline void PushClear(render_group* Group, vector4 Color)
{
	render_entry_clear* Entry = PushStruct(Group->Arena, render_entry_clear);
	Group->LastEntry = (uint8_t*) Entry;
	Entry->Header.Type = EntryType_Clear;
	Entry->Color = Color;
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
				DrawRectangle(
					BackBuffer,
					MakeRectangle(Entry->Position, Entry->Dim),
					Entry->Color.R,
					Entry->Color.G,
					Entry->Color.B
				);
				CurrentAddress += sizeof(*Entry);
				break;
			}
			case(EntryType_Bitmap):
			{
				render_entry_bitmap* Entry = (render_entry_bitmap*) Header;
				DrawBitmap(
					BackBuffer,
					Entry->Bitmap,
					Entry->Position.X,
					Entry->Position.Y
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
