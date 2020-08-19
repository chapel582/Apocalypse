#include "apocalypse_render_group.h"

#include "stb_truetype.h"

#include "apocalypse_math.h"
#include "apocalypse_rectangle.h"
#include "apocalypse_vector.h"
#include "apocalypse_bitmap.h"
#include "apocalypse_assets.h"
#include "apocalypse_particles.h"

vector2 TransformVectorToBasis(basis* Basis, vector2 Vector)
{
	vector2 Result = {};
	Result = (
		Vector.X * Basis->Axis1 + 
		Vector.Y * Basis->Axis2
	);
	return Result;
}

vector2 TransformVectorFromBasis(basis* Basis, vector2 Vector)
{
	vector2 Result = {};
	Result = (
		Vector.X * Basis->MatrixColumn1 + 
		Vector.Y * Basis->MatrixColumn2
	);
	return Result;
}

vector2 TransformPosToBasis(basis* Basis, vector2 Vector)
{
	vector2 Result = {};
	vector2 Position = Vector - Basis->Offset;
	Result = TransformVectorToBasis(Basis, Position);
	return Result;
}

vector2 TransformPosFromBasis(basis* Basis, vector2 Vector)
{
	vector2 Result = {};
	Result = TransformVectorFromBasis(Basis, Vector);
	Result += Basis->Offset;
	return Result;
}

vector2 ScreenToWorldPos(basis* WorldScreenBasis, uint32_t X, uint32_t Y)
{
	return TransformPosToBasis(WorldScreenBasis, Vector2(X, Y));
}

vector2 WorldToScreenPos(basis* WorldScreenBasis, vector2 WorldPosition)
{
	return TransformPosFromBasis(WorldScreenBasis, WorldPosition);
}

vector2 ScreenToWorldDim(basis* WorldScreenBasis, vector2 ScreenDim)
{
	return TransformVectorToBasis(WorldScreenBasis, ScreenDim);
}

vector2 WorldToScreenDim(basis* WorldScreenBasis, vector2 WorldDim)
{
	return TransformVectorFromBasis(WorldScreenBasis, WorldDim);
}

struct screen_pos_dim
{
	vector2 Pos;
	vector2 XAxis;
	vector2 YAxis;
};

screen_pos_dim GetScreenPosDim(
	basis* Basis, basis* WorldToCamera, basis* CameraToScreen
)
{
	screen_pos_dim Result = {};

	vector2 CameraSpacePos = TransformPosToBasis(WorldToCamera, Basis->Offset);
	Result.Pos = TransformPosToBasis(CameraToScreen, CameraSpacePos);

	vector2 CameraAxis = TransformVectorToBasis(WorldToCamera, Basis->Axis1);
	Result.XAxis = TransformVectorToBasis(CameraToScreen, CameraAxis);

	CameraAxis = TransformVectorToBasis(WorldToCamera, Basis->Axis2);
	Result.YAxis = TransformVectorToBasis(CameraToScreen, CameraAxis);

	return Result;
}

inline void PushBitmap(
	render_group* Group,
	loaded_bitmap* Bitmap,
	basis* Basis,
	vector4 Color
)
{
	// TODO: we might want a way to push a bitmap while only designating its 
	// CONT: dimentions in world space
	// TODO: may want a way to push a centered bitmap
	// TODO: no way to make bitmap semi-transparent! Add alpha for that if you 
	// CONT: need it
	// NOTE: basis offset should be the top left of the unrotated texture
	render_entry_bitmap* Entry = PushStruct(Group->Arena, render_entry_bitmap);
	Group->LastEntry = (uint8_t*) Entry;
	Entry->Header.Type = EntryType_Bitmap;
	Entry->Bitmap = Bitmap;
	Entry->Color = Color;

	screen_pos_dim ScreenPosDim = GetScreenPosDim(
		Basis, Group->WorldToCamera, Group->CameraToScreen
	);
	Entry->Pos = ScreenPosDim.Pos;
	Entry->XAxis = (
		((float) Entry->Bitmap->Width) * ScreenPosDim.XAxis
	);
	Entry->YAxis = (
		((float) Entry->Bitmap->Height) * ScreenPosDim.YAxis
	);
}

inline void PushCenteredBitmap(
	render_group* Group,
	loaded_bitmap* Bitmap,
	vector2 Center,
	vector2 XAxis,
	vector2 YAxis,
	vector4 Color
)
{
	// NOTE: rotation is CCW e.g. Xaxis = {0.707, 0.707}  
	// CONT: and YAxis = {-.707, 0.707} is 45 degrees CW 

	// NOTE: We need to offset the center of world space by the bmp dimensions 
	// CONT: translated to world space and scaled according to our axes
	vector2 CameraXAxis = XAxis;
	vector2 CameraYAxis = YAxis;

	vector2 BitmapDim = TransformVectorFromBasis(
		Group->WorldToCamera,
		Vector2(Bitmap->Width, Bitmap->Height)
	);

	basis Basis = MakeBasis(
		(
			Center - 
			(0.5f * CameraXAxis * BitmapDim.X) - 
			(0.5f * CameraYAxis * BitmapDim.Y)
		),
		XAxis,
		YAxis
	);
	PushBitmap(Group, Bitmap, &Basis, Color);
}

inline void PushCenteredBitmap(
	render_group* Group,
	assets* Assets,
	bitmap_handle BitmapHandle,
	vector2 Center,
	vector2 XAxis,
	vector2 YAxis,
	vector4 Color
)
{
	loaded_bitmap* Bitmap = GetBitmap(Assets, BitmapHandle);
	if(Bitmap)
	{
		PushCenteredBitmap(Group, Bitmap, Center, XAxis, YAxis, Color);
	}
}

inline void PushSizedBitmap(
	render_group* Group,
	loaded_bitmap* Bitmap,
	vector2 Center,
	vector2 SizedXAxis,
	vector2 SizedYAxis,
	vector4 Color
)
{
	// NOTE: this is for when you know the dimensions that you want the bitmap
	// CONT: to be in world space already, rather than scaling it
	// TODO: remove this commented out code
	// vector2 BmpWorldDim = ScreenToWorldDim(
	// 	Group->WorldScreenBasis, Vector2(Bitmap->Width, Bitmap->Height)
	// );
	vector2 BmpWorldDim = TransformVectorFromBasis(
		Group->CameraToScreen, Vector2(Bitmap->Width, Bitmap->Height)
	);
	vector2 CameraXAxis = TransformVectorToBasis(
		Group->WorldToCamera, SizedXAxis
	);
	vector2 CameraYAxis = TransformVectorToBasis(
		Group->WorldToCamera, SizedYAxis
	);

	/* NOTE: 
		XAxis / BmpWorldDim.X (as with Y) works b/c this is a reduction of the 
		following

		1. Get BMP dim with no scaling in world space
		2. Find ratio of axis magnitude to bmp dim 
		(this ratio tells us how much to actually scale the bmp along each axis)
		3. Normalize axis
		4. Multiply normalized axis by ratio to get percentage scaling on each
		axis

		Clearly, the normalized axis is (axis / mag) and the ratio is 
		(mag / bmp dim). We can cancel the magnitudes and save the calculation
	*/
	PushCenteredBitmap(
		Group,
		Bitmap,
		Center,
		CameraXAxis / BmpWorldDim.X,
		CameraYAxis / BmpWorldDim.Y,
		Color
	);
}

inline void PushSizedBitmap(
	render_group* Group,
	assets* Assets,
	bitmap_handle BitmapHandle,
	vector2 Center,
	vector2 SizedXAxis,
	vector2 SizedYAxis,
	vector4 Color
)
{
	loaded_bitmap* Bitmap = GetBitmap(Assets, BitmapHandle);
	if(Bitmap)
	{
		PushSizedBitmap(Group, Bitmap, Center, SizedXAxis, SizedYAxis, Color);
	}
}

inline void PushSizedBitmap(
	render_group* Group,
	assets* Assets,
	bitmap_handle BitmapHandle,
	vector2 Center,
	vector2 XYAxes,
	vector4 Color
)
{
	loaded_bitmap* Bitmap = GetBitmap(Assets, BitmapHandle);
	if(Bitmap)
	{
		PushSizedBitmap(
			Group,
			Bitmap,
			Center,
			Vector2(XYAxes.X, 0.0f),
			Vector2(0.0f, XYAxes.Y),
			Color
		);
	}
}

void PushParticles(
	render_group* Group, assets* Assets, particle_system* ParticleSystem
)
{
	particle* Particle = &ParticleSystem->Particles[0];
	for(uint32_t Index = 0; Index < ParticleSystem->ParticleCount; Index++)
	{
		if(Particle->State != ParticleState_Inactive)
		{
			PushSizedBitmap(
				Group,
				Assets,
				ParticleSystem->ParticleBitmap,
				Particle->Pos,
				Vector2(Particle->Dim.X, 0.0f),
				Vector2(0.0f, Particle->Dim.Y),
				Particle->Color
			);
		}
		Particle++;
	}
}

inline void PushGlyph(
	render_group* Group,
	assets* Assets,
	font_handle FontHandle,
	uint32_t CodePoint,
	vector2 BottomLeft,
	vector2 XAxis,
	vector2 YAxis,
	vector4 Color
)
{
	// TODO: consider if there's a better way to handle avoiding white space 
	// CONT: than spending two comparisons for every push
	if(CodePoint == ' ' || CodePoint == '\n')
	{
		return;
	}

	loaded_glyph* Glyph = GetGlyph(Assets, FontHandle, CodePoint);
	if(Glyph)
	{
		basis Basis = MakeBasis(BottomLeft, XAxis, YAxis);
		PushBitmap(
			Group, &Glyph->Bitmap, &Basis, Color
		);
	}
}

inline void PushOffsetGlyph(
	render_group* Group,
	assets* Assets,
	font_handle FontHandle,
	stbtt_fontinfo* Font,
	uint32_t CodePoint,
	float Scale,
	vector2 XAxis,
	vector2 YAxis,
	vector2 LeftBaselinePoint,
	vector2 Offset,
	vector4 Color
)
{
	// NOTE: function pulled out for use in PushText only
	int X0, Y0, X1, Y1;
	stbtt_GetCodepointBox(Font, CodePoint, &X0, &Y0, &X1, &Y1);
	PushGlyph(
		Group,
		Assets,
		FontHandle,
		CodePoint,
		LeftBaselinePoint + Offset + (Scale * Vector2(X0, Y0)),
		XAxis,
		YAxis,
		Color
	);
}

push_text_result PushText(
	render_group* Group,
	assets* Assets,
	font_handle FontHandle,
	uint32_t* CodePoints,
	uint32_t CodePointCount,
	float FontHeight, // NOTE: font height in world units
	vector2 LeftBaselinePoint,
	vector4 Color
)
{
	push_text_result Result = {};

	loaded_font* LoadedFont = GetFont(Assets, FontHandle);
	if(LoadedFont == NULL)
	{
		goto end;
	}

	vector2 CameraHeight = TransformVectorFromBasis(
		Group->WorldToCamera, Vector2(0.0f, FontHeight)
	);
	vector2 PixelHeightV2 = TransformVectorFromBasis(
		Group->CameraToScreen, CameraHeight
	);
	float PixelHeight = PixelHeightV2.Y;

	// NOTE: everything after this point is in pixel space
	stbtt_fontinfo* Font = &LoadedFont->StbFont;
	int Ascent = 0;
	int Descent = 0;
	int LineGap = 0;
	float Scale = stbtt_ScaleForPixelHeight(Font, PixelHeight);
	stbtt_GetFontVMetrics(Font, &Ascent, &Descent, &LineGap);
	float YAdvance = Scale * (Ascent - Descent + LineGap);
	// TODO: is there a way to get LPad programatically using stbtt?
	float LPad = 2.0f; 
	vector2 Offset = Vector2(LPad, 0.0f);
	
	uint32_t* CodePointPtr = CodePoints;
	vector2 YAxis = Vector2(0.0f, PixelHeight / LoadedFont->PixelHeight);
	vector2 XAxis = -1.0f * Perpendicular(YAxis); // NOTE: want CW perp
	for(uint32_t Index = 0; Index < CodePointCount - 1; Index++)
	{
		uint32_t CodePoint = *CodePointPtr;
		// NOTE: X0 is a signed value indicating the offset from the standard 
		// CONT: advance / kern adjustment
		// NOTE: Y0 is a signed value indicating the ascent/descent from the 
		// CONT: baseline
		if(CodePoint == '\n')
		{
			Offset.X = LPad;
			Offset.Y -= YAdvance;
		}
		else
		{
			PushOffsetGlyph(
				Group,
				Assets,
				FontHandle,
				Font,
				CodePoint,
				Scale,
				XAxis,
				YAxis,
				LeftBaselinePoint,
				Offset,
				Color
			);
			int Advance, Lsb;
			stbtt_GetCodepointHMetrics(Font, CodePoint, &Advance, &Lsb);
			int Kern = stbtt_GetCodepointKernAdvance(
				Font, CodePoint, *(CodePointPtr + 1)
			);
			Offset.X += Scale * (Advance + Kern);
		}
		
		CodePointPtr++;
	}
	// NOTE: push the last glyph
	uint32_t CodePoint = *CodePointPtr;
	PushOffsetGlyph(
		Group,
		Assets,
		FontHandle,
		Font,
		CodePoint,
		Scale,
		XAxis,
		YAxis,
		LeftBaselinePoint,
		Offset,
		Color
	);

	{
		int Advance, Lsb;
		stbtt_GetCodepointHMetrics(Font, CodePoint, &Advance, &Lsb);
		// int Kern = stbtt_GetCodepointKernAdvance(
		// 	Font, CodePoint, *(CodePointPtr + 1)
		// );
		// NOTE: no kerning b/c next character should be white space
		Offset.X += Scale * Advance;
	}

	Result.Code = PushText_Success;
	Result.Offset = Offset;
	goto end;

end:
	return Result;
}

push_text_result PushText(
	render_group* Group,
	assets* Assets,
	font_handle FontHandle,
	char* CodePoints,
	uint32_t MaxCodePointCount,
	float FontHeight, // NOTE: font height in world units
	vector2 LeftBaselinePoint,
	vector4 Color,
	memory_arena* FrameArena // NOTE: this function will leak if you don't
	// CONT: regularly clear the arena. Hence, FrameArena
)
{
	/* NOTE: 
	This function pushes text with a baseline at LeftBaselinePoint.Y startin at
	LeftBaselinePoint.X
	*/

	// NOTE: this function is slower than just keeping your text in 4-byte
	// CONT: codepoints all the time. If you need speed, store your data as 
	// CONT: 4-byte codepoints and use the codepoint version of this function
	uint32_t* TempBuffer = PushArray(
		FrameArena, MaxCodePointCount + 1, uint32_t
	);
	char* Char = CodePoints;
	uint32_t* WriteIndex = 0;
	uint32_t Index;
	for(Index = 0; Index < MaxCodePointCount; Index++)
	{
		if(*Char == 0)
		{
			break;
		}
		TempBuffer[Index] = (uint32_t) *Char;
		Char++;
	}
	TempBuffer[Index] = 0;

	return PushText(
		Group,
		Assets,
		FontHandle,
		TempBuffer,
		Index,
		FontHeight,
		LeftBaselinePoint,
		Color
	);
}

push_text_result PushTextTopLeft(
	render_group* Group,
	assets* Assets,
	font_handle FontHandle,
	char* CodePoints,
	uint32_t MaxCodePointCount,
	float FontHeight, // NOTE: font height in world units
	vector2 TopLeft,
	vector4 Color,
	memory_arena* FrameArena // NOTE: this function will leak if you don't
	// CONT: regularly clear the arena. Hence, FrameArena
)
{
	/* NOTE: 
		this function is for pushing text that aligns with a top 
		left corner. e.g. the highest ascending glyph won't go exceed TopLeft.Y,
		and  the farthest back glyph won't go beyond the TopLeft.X
	*/
	push_text_result Result = {};

	loaded_font* LoadedFont = GetFont(Assets, FontHandle); 
	if(LoadedFont == NULL)
	{
		goto end;
	}

	vector2 CameraHeight = TransformVectorFromBasis(
		Group->WorldToCamera, Vector2(0.0f, FontHeight)
	);
	vector2 PixelHeightV2 = TransformVectorFromBasis(
		Group->CameraToScreen, CameraHeight
	);
	float PixelHeight = PixelHeightV2.Y;

	// NOTE: everything after this point is in pixel space
	stbtt_fontinfo* Font = &LoadedFont->StbFont;
	// TODO: Get LPad programatically using stbtt?
	float LPad = 2.0f; 
	int Ascent = 0;
	float Scale = stbtt_ScaleForPixelHeight(Font, PixelHeight);
	stbtt_GetFontVMetrics(Font, &Ascent, NULL, NULL);
	float ScaledAscent = Scale * Ascent;
	vector2 AscentWorldHeight = TransformVectorToBasis(
		Group->CameraToScreen, Vector2(0.0f, ScaledAscent)
	);
	vector2 AscentCameraHeight = TransformVectorToBasis(
		Group->WorldToCamera, AscentWorldHeight
	);
	vector2 Baseline = TopLeft - AscentCameraHeight;

	Result = PushText(
		Group,
		Assets,
		FontHandle,
		CodePoints,
		MaxCodePointCount,
		FontHeight,
		Baseline,
		Color,
		FrameArena
	);
	goto end;

end:
	return Result;
}

push_text_result PushTextCentered(
	render_group* Group,
	assets* Assets,
	font_handle FontHandle,
	char* CodePoints,
	uint32_t MaxCodePointCount,
	float FontHeight, 
	vector2 Center,
	vector4 Color,
	memory_arena* FrameArena 
)
{
	// NOTE: FontHeight is in world units
	// NOTE: Center.X will be the middle of the rendered text. 
	// NOTE: Center.Y - FontHeight / 2.0f will be the baseline
	// NOTE: this function will leak if you don't regularly clear the arena.
	// CONT: Hence, use FrameArena
	push_text_result Result = {};

	loaded_font* LoadedFont = GetFont(Assets, FontHandle);
	if(LoadedFont == NULL)
	{
		goto end;
	}

	stbtt_fontinfo* Font = &LoadedFont->StbFont;
	vector2 CameraHeight = TransformVectorFromBasis(
		Group->WorldToCamera, Vector2(0.0f, FontHeight)
	);
	vector2 PixelHeightV2 = TransformVectorFromBasis(
		Group->CameraToScreen, CameraHeight
	);
	float PixelHeight = PixelHeightV2.Y;

	// TODO: is there a way to get LPad programatically using stbtt?
	float LPad = 2.0f; 
	float Scale = stbtt_ScaleForPixelHeight(Font, PixelHeight);
	
	char* CodePointPtr = CodePoints;
	float PixelWidth = LPad;
	float MaxWidth = 0.0f;
	uint32_t CodePointIndex;
	for(
		CodePointIndex = 0;
		CodePointIndex < MaxCodePointCount;
		CodePointIndex++
	)
	{
		uint32_t CodePoint = (uint32_t) *CodePointPtr;
		if(CodePoint == 0)
		{
			if(PixelWidth > MaxWidth)
			{
				MaxWidth = PixelWidth;
			}
			break;
		}
		else if(CodePoint == '\n')
		{
			if(PixelWidth > MaxWidth)
			{
				MaxWidth = PixelWidth;
			}
			PixelWidth = LPad;
		}
		else
		{
			int Advance, Lsb;
			stbtt_GetCodepointHMetrics(Font, CodePoint, &Advance, &Lsb);
			int Kern = stbtt_GetCodepointKernAdvance(
				Font, CodePoint, (uint32_t) *(CodePointPtr + 1)
			);
			PixelWidth += Scale * (Advance + Kern);
		}
		
		CodePointPtr++;
	}
	if(CodePointIndex == MaxCodePointCount && MaxWidth == 0.0f)
	{
		MaxWidth = PixelWidth;
	}

	vector2 BaselineOffsetFromCenter = TransformVectorToBasis(
		Group->CameraToScreen, Vector2(MaxWidth / 2.0f, FontHeight / 2.0f)
	);
	vector2 BaselineOffsetFromCenterWorld = TransformVectorToBasis(
		Group->WorldToCamera, BaselineOffsetFromCenter
	);
	vector2 Baseline = Center - BaselineOffsetFromCenterWorld;

	Result = PushText(
		Group,
		Assets,
		FontHandle,
		CodePoints,
		MaxCodePointCount,
		FontHeight,
		Baseline,
		Color,
		FrameArena
	);

	goto end;

end:
	return Result;
}

inline void PushRect(render_group* Group, rectangle Rectangle, vector4 Color)
{
	// NOTE: Basis should be the bottom left of the unrotated rect

	render_entry_rectangle* Entry = PushStruct(
		Group->Arena, render_entry_rectangle
	);
	Group->LastEntry = (uint8_t*) Entry;
	Entry->Header.Type = EntryType_Rectangle;
	Entry->Color = Color;

	basis Basis = MakeBasis(
		GetBottomLeft(Rectangle), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f)
	);

	screen_pos_dim ScreenPosDim = GetScreenPosDim(
		&Basis, Group->WorldToCamera, Group->CameraToScreen
	);
	Entry->Pos = ScreenPosDim.Pos;
	Entry->XAxis = ((float) Rectangle.Dim.X) * ScreenPosDim.XAxis;
	Entry->YAxis = ((float) Rectangle.Dim.Y) * ScreenPosDim.YAxis;
}

inline void PushClear(render_group* Group, vector4 Color)
{
	render_entry_clear* Entry = PushStruct(Group->Arena, render_entry_clear);
	Group->LastEntry = (uint8_t*) Entry;
	Entry->Header.Type = EntryType_Clear;
	Entry->Color = Color;
}

bool IsInRotatedQuad(
	vector2 PointOffsetFromOrigin,
	vector2 XAxis,
	vector2 YAxis,
	vector2 XAxisPerp,
	vector2 YAxisPerp
)
{
	float Edge0 = Inner(PointOffsetFromOrigin, -1 * XAxisPerp);
	float Edge1 = Inner(PointOffsetFromOrigin - XAxis, -1 * YAxisPerp);
	float Edge2 = Inner(
		PointOffsetFromOrigin - XAxis - YAxis, XAxisPerp
	);
	float Edge3 = Inner(PointOffsetFromOrigin - YAxis, YAxisPerp);
	return (Edge0 < 0) && (Edge1 < 0) && (Edge2 < 0) && (Edge3 < 0);
}

inline vector4 Unpack4x8(uint32_t Packed)
{
	return Vector4(
		(float) ((Packed >> 16) & 0xFF),
		(float) ((Packed >> 8) & 0xFF),
		(float) (Packed & 0xFF),
		(float) ((Packed >> 24) & 0xFF)
	);
}

inline vector4 Rgb255ToNormalColor(vector4 C)
{
	vector4 Result;

	Result.R = (float) (C.R / 255.0f);
	Result.G = (float) (C.G / 255.0f);
	Result.B = (float) (C.B / 255.0f);
	Result.A = (float) (C.A / 255.0f);

	return Result;
}

inline vector4 NormalColorToRgb255(vector4 C)
{
	vector4 Result;

	Result.R = 255.0f * C.R;
	Result.G = 255.0f * C.G;
	Result.B = 255.0f * C.B;
	Result.A = 255.0f * C.A;

	return Result;
}

void DrawRectangle(
	loaded_bitmap* Buffer, 
	rectangle Rectangle,
	vector4 vColor 
)
{
	TIMED_BLOCK();

	int32_t MinX = RoundFloat32ToInt32(Rectangle.Min.X);
	int32_t MinY = RoundFloat32ToInt32(Rectangle.Min.Y);
	int32_t MaxX = RoundFloat32ToInt32(Rectangle.Min.X + Rectangle.Dim.X);
	int32_t MaxY = RoundFloat32ToInt32(Rectangle.Min.Y + Rectangle.Dim.Y);

	if(MinX < 0)
	{
		MinX = 0;
	}
	else if(MinX > Buffer->Width)
	{
		MinX = Buffer->Width;
	}
	if(MaxX < 0)
	{
		MaxX = 0;
	}
	else if(MaxX > Buffer->Width)
	{
		MaxX = Buffer->Width;
	}
	if(MinY < 0)
	{
		MinY = 0;
	}
	else if(MinY > Buffer->Height)
	{
		MinY = Buffer->Height;
	}
	if(MaxY < 0)
	{
		MaxY = 0;
	}
	else if(MaxY > Buffer->Height)
	{
		MaxY = Buffer->Height;
	}

	uint32_t Color = (
		(RoundFloat32ToInt32(vColor.R * 0xFF) << 16) |
		(RoundFloat32ToInt32(vColor.G * 0xFF) << 8) |
		(RoundFloat32ToInt32(vColor.B * 0xFF)) |
		(RoundFloat32ToInt32(vColor.A * 0xFF) << 24) 
	);

	uint8_t* Row = (
		((uint8_t*) Buffer->Memory) + 
		MinX * BYTES_PER_PIXEL +
		MinY * Buffer->Pitch 
	);

	for(int Y = MinY; Y < MaxY; Y++)
	{
		uint32_t* Pixel = (uint32_t*) Row; 
		for(int X = MinX; X < MaxX; X++)
		{
			*Pixel++ = Color;
		}
		Row += Buffer->Pitch;
	}
}

void DrawRectangleSlowly(
	loaded_bitmap* Buffer, 
	vector2 Origin,
	vector2 XAxis,
	vector2 YAxis,
	vector4 Color
)
{
	TIMED_BLOCK();
	float fMinX = (float) Buffer->Width;
	float fMinY = (float) Buffer->Height;
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
	else if(MinX > Buffer->Width)
	{
		MinX = Buffer->Width;
	}
	if(MaxX < 0)
	{
		MaxX = 0;
	}
	else if(MaxX > Buffer->Width)
	{
		MaxX = Buffer->Width;
	}
	if(MinY < 0)
	{
		MinY = 0;
	}
	else if(MinY > Buffer->Height)
	{
		MinY = Buffer->Height;
	}
	if(MaxY < 0)
	{
		MaxY = 0;
	}
	else if(MaxY > Buffer->Height)
	{
		MaxY = Buffer->Height;
	}

	vector2 XAxisPerp = Perpendicular(XAxis);
	vector2 YAxisPerp = Perpendicular(YAxis);

	uint8_t* Row = (
		((uint8_t*) Buffer->Memory) + 
		MinX * BYTES_PER_PIXEL +
		MinY * Buffer->Pitch 
	);
	for(int Y = MinY; Y < MaxY; Y++)
	{
		uint32_t* Pixel = (uint32_t*) Row; 
		for(int X = MinX; X < MaxX; X++)
		{
			vector2 Point = Vector2(X, Y);
			vector2 PointOffsetFromOrigin = Point - Origin;

			bool InRotatedQuad = IsInRotatedQuad(
				PointOffsetFromOrigin,
				XAxis,
				YAxis,
				XAxisPerp,
				YAxisPerp
			);
			if(InRotatedQuad)
			{
				uint32_t Source = *Pixel;
				vector4 v4Source = Rgb255ToNormalColor(Unpack4x8(Source));

				vector4 BlendedColor = Lerp(v4Source, Color.A, Color);
				vector4 BlendedColorRgb255 = NormalColorToRgb255(BlendedColor);

				*Pixel = (
					(((uint32_t) 0xFF) << 24) | 
					(((uint32_t) BlendedColorRgb255.R) << 16) |
					(((uint32_t) BlendedColorRgb255.G) << 8) |
					((uint32_t) BlendedColorRgb255.B)
				);
				// *Pixel = Color;
			}
			Pixel++;
		}
		Row += Buffer->Pitch;
	}
}

// TODO: delete me
void DrawBitmap(
	loaded_bitmap* Buffer,
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
	uint32_t* SourceRow = (
		((uint32_t*) Bitmap->Memory) + Bitmap->Width * (Bitmap->Height - 1)
	);
	SourceRow += -SourceOffsetY * Bitmap->Width + SourceOffsetX;
	uint8_t* DestRow = (
		(uint8_t*) Buffer->Memory +
		MinX * BYTES_PER_PIXEL + 
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

inline __m128 VectorizedUnpack4x8(uint32_t Packed)
{
	__m128 Result;
	float* Element = (float*) &Result;
	*Element++ = (float) ((Packed >> 16) & 0xFF);
	*Element++ = (float) ((Packed >> 8) & 0xFF);
	*Element++ = (float) (Packed & 0xFF);
	*Element++ = (float) ((Packed >> 24) & 0xFF);
	return Result;
}

struct bilinear_sample
{
	uint32_t A, B, C, D;
};

inline bilinear_sample BilinearSample(
	loaded_bitmap* Texture, int32_t X, int32_t Y
)
{
	bilinear_sample Result;
	
	uint8_t* TexelPtr = (
		((uint8_t*)Texture->Memory) + Y * Texture->Pitch + X * sizeof(uint32_t)
	);
	Result.A = *(uint32_t*) (TexelPtr);
	Result.B = *(uint32_t*) (TexelPtr + sizeof(uint32_t));
	Result.C = *(uint32_t*) (TexelPtr + Texture->Pitch);
	Result.D = *(uint32_t*) (TexelPtr + Texture->Pitch + sizeof(uint32_t));

	return Result;
}

/*
TODO: 
	We probably need a totally different, and simpler, lighting strategy.
	Still, making a note here for a the environment map strategy. Use the Y
	component of the normal to figure out which map to look up into, use X and 
	Z normals in conjunction with position in space to look up into that 2D
	env map.
*/

#define GET_FLOAT_FROM_MM128(Variable, Index) (*(((float*) Variable) + Index))
void DrawBitmapQuickly(
	loaded_bitmap* Buffer, 
	vector2 Origin,
	vector2 XAxis,
	vector2 YAxis,
	loaded_bitmap* Texture,
	vector4 Color
)
{
	TIMED_BLOCK();

	__m128 WideOne = _mm_set1_ps(1.0f);
	__m128 WideZero = _mm_set1_ps(0.0f);
	__m128 Wide255 = _mm_set1_ps(255.0f);
	__m128 WideInv255 = _mm_set1_ps(1.0f / 255.0f);
	__m128 WideColor = *((__m128*) &Color);

	float fMinX = (float) Buffer->Width;
	float fMinY = (float) Buffer->Height;
	float fMaxX = 0;
	float fMaxY = 0;

	float InvWidthMax = 1.0f / ((float) Buffer->Width);
	float InvHeightMax = 1.0f / ((float) Buffer->Height);

	float XAxisLength = Magnitude(XAxis);
	float YAxisLength = Magnitude(YAxis);
	// NOTE: NxAxis and NyAxis are projections of each axis across the other
	vector2 NxAxis = (YAxisLength / XAxisLength) * XAxis;
	vector2 NyAxis = (XAxisLength / YAxisLength) * YAxis;
	float NzScale = 0.5f * (XAxisLength + YAxisLength);

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
	else if(MinX > Buffer->Width)
	{
		MinX = Buffer->Width;
	}
	if(MaxX < 0)
	{
		MaxX = 0;
	}
	else if(MaxX > Buffer->Width)
	{
		MaxX = Buffer->Width;
	}
	if(MinY < 0)
	{
		MinY = 0;
	}
	else if(MinY > Buffer->Height)
	{
		MinY = Buffer->Height;
	}
	if(MaxY < 0)
	{
		MaxY = 0;
	}
	else if(MaxY > Buffer->Height)
	{
		MaxY = Buffer->Height;
	}

	float InvXAxisLengthSq = 1.0f / MagnitudeSquared(XAxis);
	float InvYAxisLengthSq = 1.0f / MagnitudeSquared(YAxis);
	vector2 XAxisOverLengthSquared = InvXAxisLengthSq * XAxis;
	vector2 YAxisOverLengthSquared = InvYAxisLengthSq * YAxis;

	vector2 XAxisPerp = Perpendicular(XAxis);
	vector2 YAxisPerp = Perpendicular(YAxis);

	uint8_t* Row = (
		((uint8_t*) Buffer->Memory) + 
		MinX * BYTES_PER_PIXEL +
		MinY * Buffer->Pitch 
	);
	{
		TIMED_BLOCK((MaxX - MinX) * (MaxY - MinY));
		for(int Y = MinY; Y < MaxY; Y++)
		{
			uint32_t* Pixel = (uint32_t*) Row;
			for(int X = MinX; X < MaxX; X++)
			{
				vector2 Point = Vector2(X, Y);
				vector2 PointOffsetFromOrigin = Point - Origin;
				// NOTE: U and V are calculated this way because 
				// CONT: Inner(PointOffsetFromOrigin, XAxis) is the projection of 
				// CONT: X, Y along the XAxis in pixels, scaled by XAxis mag,
				// CONT: so we divide by the magnitude twice, which is MagSquared
				float U = Inner(PointOffsetFromOrigin, XAxisOverLengthSquared);
				float V = Inner(PointOffsetFromOrigin, YAxisOverLengthSquared);
				if(
					(U >= 0.0f) && 
					(U <= 1.0f) &&
					(V >= 0.0f) &&
					(V <= 1.0f)
				)
				{
					// TODO: formalize texture boundaries
					// NOTE: U and V are then used to find the sample within the 
					// CONT: texture 
					float tX = U * ((float)(Texture->Width - 2));
					float tY = V * ((float)(Texture->Height - 2));
					
					int32_t iX = (int32_t) tX;
					int32_t iY = (int32_t) tY;

					float fX = tX - ((float) iX);
					float fY = tY - ((float) iY);
					__m128 WideFX = _mm_set1_ps(fX);
					__m128 OneMinusFX = _mm_sub_ps(WideOne, WideFX);
					__m128 WideFY = _mm_set1_ps(fY);
					__m128 OneMinusFY = _mm_sub_ps(WideOne, WideFY);

					// NOTE: lerp based on texels below above and to the right
					// bilinear_sample Sample = BilinearSample(Texture, iX, iY);
					uint8_t* TexelPtr = (
						((uint8_t*)Texture->Memory) + 
						iY * Texture->Pitch + 
						iX * sizeof(uint32_t)
					);
					uint32_t SampleA = *(uint32_t*) (TexelPtr);
					uint32_t SampleB = *(uint32_t*) (TexelPtr + sizeof(uint32_t));
					uint32_t SampleC = *(uint32_t*) (TexelPtr + Texture->Pitch);
					uint32_t SampleD = *(uint32_t*) (
						TexelPtr + Texture->Pitch + sizeof(uint32_t)
					);

					// TODO: Color.a!!
					__m128 TexelA = VectorizedUnpack4x8(SampleA);
					__m128 TexelB = VectorizedUnpack4x8(SampleB);
					__m128 TexelC = VectorizedUnpack4x8(SampleC);
					__m128 TexelD = VectorizedUnpack4x8(SampleD);

					// NOTE: normalized texel colors
					TexelA = _mm_mul_ps(TexelA, WideInv255);
					TexelB = _mm_mul_ps(TexelB, WideInv255);
					TexelC = _mm_mul_ps(TexelC, WideInv255);
					TexelD = _mm_mul_ps(TexelD, WideInv255);

					// NOTE: Lerp to bilinear blend texel
					__m128 Texel = _mm_add_ps(
						_mm_mul_ps(
							OneMinusFY,
							_mm_add_ps(
								_mm_mul_ps(OneMinusFX, TexelA),
								_mm_mul_ps(WideFX, TexelB) 
							)
						),
						_mm_mul_ps(
							WideFY,
							_mm_add_ps(
								_mm_mul_ps(OneMinusFX, TexelC),
								_mm_mul_ps(WideFX, TexelD)
							)
						)
					);

					// NOTE: mask with color
					Texel = _mm_mul_ps(Texel, WideColor);

					// NOTE: Clamping
					// NOTE: the point of the clamping is to normalize the color 
					// CONT: before converting to 255 color
					Texel = _mm_min_ps(_mm_max_ps(Texel, WideZero), WideOne);

					// NOTE: blend with source
					__m128 NormalizedAlpha = _mm_set1_ps(
						GET_FLOAT_FROM_MM128(&Texel, 3)
					);
					__m128 OneMinusNormalizedAlpha = _mm_sub_ps(
						WideOne, NormalizedAlpha
					);
					__m128 SourceColor = VectorizedUnpack4x8(*Pixel);
					SourceColor = _mm_mul_ps(SourceColor, WideInv255);
					__m128 NormalFinalColor = _mm_add_ps(
						_mm_mul_ps(OneMinusNormalizedAlpha, SourceColor),
						_mm_mul_ps(NormalizedAlpha, Texel)
					);
					__m128 FinalColor255 = _mm_mul_ps(NormalFinalColor, Wide255);

					float R = GET_FLOAT_FROM_MM128(&FinalColor255, 0);
					float G = GET_FLOAT_FROM_MM128(&FinalColor255, 1);
					float B = GET_FLOAT_FROM_MM128(&FinalColor255, 2);
					*Pixel = (
						(((uint32_t) 0xFF) << 24) | 
						(((uint32_t) (R + 0.5f)) << 16) |
						(((uint32_t) (G + 0.5f)) << 8) |
						((uint32_t) (B + 0.5f))
					);
				}
				Pixel++;
			}
			Row += Buffer->Pitch;
		}
	}	
}

void Clear(loaded_bitmap* Buffer, vector4 Color)
{
	DrawRectangle(
		Buffer,
		MakeRectangle(
			Vector2(0.0f, 0.0f),
			Vector2((float) Buffer->Width, (float) Buffer->Height)
		),
		Color
	);
}

void RenderGroupToOutput(
	render_group* RenderGroup, loaded_bitmap* Target
)
{
	TIMED_BLOCK();
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
					Target, 
					Vector4(
						Entry->Color.R, Entry->Color.G, Entry->Color.B, 1.0f
					)
				);
				CurrentAddress += sizeof(*Entry);
				break;
			}
			case(EntryType_Rectangle):
			{
				render_entry_rectangle* Entry = (render_entry_rectangle*) (
					Header
				);

				DrawRectangleSlowly(
					Target,
					Entry->Pos,
					Entry->XAxis,
					Entry->YAxis,
					Entry->Color
				);
				CurrentAddress += sizeof(*Entry);
				break;
			}
			case(EntryType_Bitmap):
			{
				render_entry_bitmap* Entry = (render_entry_bitmap*) Header;

				DrawBitmapQuickly(
					Target,
					Entry->Pos,
					Entry->XAxis,
					Entry->YAxis,
					Entry->Bitmap,
					Entry->Color
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
