#include "apocalypse_render_group.h"

#include "apocalypse_math.h"
#include "apocalypse_rectangle.h"
#include "apocalypse_vector.h"

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

inline void PushBitmap(
	render_group* Group,
	loaded_bitmap* Bitmap,
	basis Basis,
	vector4 Color,
	loaded_bitmap* NormalMap,
	environment_map* Top,
	environment_map* Middle,
	environment_map* Bottom
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
	Entry->Basis = Basis;
	Entry->Bitmap = Bitmap;
	Entry->Color = Color;
	Entry->NormalMap = NormalMap;
	Entry->Top = Top;
	Entry->Middle = Middle;
	Entry->Bottom = Bottom;
}

inline void PushCenteredBitmap(
	render_group* Group,
	loaded_bitmap* Bitmap,
	vector2 Center,
	vector2 XAxis,
	vector2 YAxis,
	vector4 Color,
	loaded_bitmap* NormalMap,
	environment_map* Top,
	environment_map* Middle,
	environment_map* Bottom
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

	PushBitmap(
		Group,
		Bitmap,
		MakeBasis(
			(
				Center - 
				(0.5f * CameraXAxis * BitmapDim.X) - 
				(0.5f * CameraYAxis * BitmapDim.Y)
			),
			XAxis,
			YAxis
		),
		Color,
		NormalMap,
		Top,
		Middle,
		Bottom
	);
}

inline void PushSizedBitmap(
	render_group* Group,
	loaded_bitmap* Bitmap,
	vector2 Center,
	vector2 SizedXAxis,
	vector2 SizedYAxis,
	vector4 Color,
	loaded_bitmap* NormalMap,
	environment_map* Top,
	environment_map* Middle,
	environment_map* Bottom
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
		Color,
		NormalMap,
		Top,
		Middle,
		Bottom
	);
}

// TODO: make a push rect that doesn't require pushing a basis and will make it based on the rect you push
inline void PushRect(
	render_group* Group, basis Basis, rectangle Rectangle, vector4 Color
)
{
	// NOTE: Basis should be the top left of the unrotated rect
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

void DrawRectangle(
	loaded_bitmap* Buffer, 
	rectangle Rectangle,
	vector4 vColor 
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
	vector4 ColorV
)
{
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

	uint32_t Color = (
		(RoundFloat32ToInt32(ColorV.R * 0xFF) << 16) |
		(RoundFloat32ToInt32(ColorV.G * 0xFF) << 8) |
		RoundFloat32ToInt32(ColorV.B * 0xFF)
	);

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
				*Pixel = Color;
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

void DrawBitmapQuickly(
	loaded_bitmap* Buffer, 
	vector2 Origin,
	vector2 XAxis,
	vector2 YAxis,
	loaded_bitmap* Texture,
	vector4 Color
)
{
	BEGIN_TIMED_BLOCK(DrawBitmapQuickly);

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
	for(int Y = MinY; Y < MaxY; Y++)
	{
		uint32_t* Pixel = (uint32_t*) Row;
		for(int X = MinX; X < MaxX; X++)
		{
			BEGIN_TIMED_BLOCK(TestPixel);
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
				BEGIN_TIMED_BLOCK(FillPixel);
				// TODO: formalize texture boundaries
				// NOTE: U and V are then used to find the sample within the 
				// CONT: texture 
				float tX = U * ((float)(Texture->Width - 2));
				float tY = V * ((float)(Texture->Height - 2));
				
				int32_t iX = (int32_t) tX;
				int32_t iY = (int32_t) tY;

				float fX = tX - ((float) iX);
				float fY = tY - ((float) iY);

				ASSERT((iX >= 0) && (iX < Texture->Width));
				ASSERT((iY >= 0) && (iY < Texture->Height));

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
				vector4 TexelA = Unpack4x8(SampleA);
				vector4 TexelB = Unpack4x8(SampleB);
				vector4 TexelC = Unpack4x8(SampleC);
				vector4 TexelD = Unpack4x8(SampleD);

				// NOTE: normalized texel colors
				TexelA = Rgb255ToNormalColor(TexelA);
				TexelB = Rgb255ToNormalColor(TexelB);
				TexelC = Rgb255ToNormalColor(TexelC);
				TexelD = Rgb255ToNormalColor(TexelD);

				// NOTE: Lerp to blend texel
				float OneMinusLerpX = 1.0f - fX;
				vector4 Texel = (
					((1.0f - fY) * (OneMinusLerpX * TexelA + fX * TexelB)) + 
					(fY * (OneMinusLerpX * TexelC + fX * TexelD))
				);

				Texel = Hadamard(Texel, Color);
				// NOTE: the point of the clamping is to normalize the color 
				// CONT: before converting to 255 color
				Texel.A = Clamp01(Texel.A);
				Texel.R = Clamp01(Texel.R);
				Texel.G = Clamp01(Texel.G);
				Texel.B = Clamp01(Texel.B);

				float NormalizedAlpha = Texel.A;
				float OneMinusNormalizedAlpha = 1.0f - NormalizedAlpha; 
				
				vector4 SourceColor;
				SourceColor.R = (float) ((((*Pixel >> 16) & 0xFF)) / 255.0f);
				SourceColor.G = (float) ((((*Pixel >> 8) & 0xFF)) / 255.0f);
				SourceColor.B = (float) ((((*Pixel) & 0xFF)) / 255.0f);

				vector4 NormalFinalColor = (
					(OneMinusNormalizedAlpha * SourceColor) + 
					(NormalizedAlpha * Texel)
				);
				ASSERT(
					NormalFinalColor.R >= 0.0f && NormalFinalColor.R <= 1.0f
				);
				ASSERT(
					NormalFinalColor.G >= 0.0f && NormalFinalColor.G <= 1.0f
				);
				ASSERT(
					NormalFinalColor.B >= 0.0f && NormalFinalColor.B <= 1.0f
				);
				vector4 FinalColor = NormalColorToRgb255(NormalFinalColor);

				*Pixel = (
					(((uint32_t) 0xFF) << 24) | 
					(((uint32_t) (FinalColor.R + 0.5f)) << 16) |
					(((uint32_t) (FinalColor.G + 0.5f)) << 8) |
					((uint32_t) (FinalColor.B + 0.5f))
				);
				END_TIMED_BLOCK(FillPixel);
			}
			Pixel++;
			END_TIMED_BLOCK(TestPixel);
		}
		Row += Buffer->Pitch;
	}

	END_TIMED_BLOCK(DrawBitmapQuickly);
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
	// vector2 WorldPos = TransformPosFromBasis(Basis, EntryPosition);
	// vector2 CameraTopLeft = WorldPos - CameraPos;

	// return TransformPosToBasis(WorldScreenBasis, CameraTopLeft);
	screen_pos_dim Result = {};

	vector2 CameraSpacePos = TransformPosToBasis(WorldToCamera, Basis->Offset);
	Result.Pos = TransformPosToBasis(CameraToScreen, CameraSpacePos);

	vector2 CameraAxis = TransformVectorToBasis(WorldToCamera, Basis->Axis1);
	Result.XAxis = TransformVectorToBasis(CameraToScreen, CameraAxis);

	CameraAxis = TransformVectorToBasis(WorldToCamera, Basis->Axis2);
	Result.YAxis = TransformVectorToBasis(CameraToScreen, CameraAxis);

	return Result;
}

void RenderGroupToOutput(
	render_group* RenderGroup, loaded_bitmap* Target
)
{
	BEGIN_TIMED_BLOCK(RenderGroupToOutput);
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

				screen_pos_dim ScreenPosDim = GetScreenPosDim(
					&Entry->Basis,
					RenderGroup->WorldToCamera,
					RenderGroup->CameraToScreen
				);

				DrawRectangleSlowly(
					Target,
					ScreenPosDim.Pos,
					Entry->Dim.X * ScreenPosDim.XAxis,
					Entry->Dim.Y * ScreenPosDim.YAxis,
					Entry->Color
				);
				CurrentAddress += sizeof(*Entry);
				break;
			}
			case(EntryType_Bitmap):
			{
				render_entry_bitmap* Entry = (render_entry_bitmap*) Header;

				screen_pos_dim ScreenPosDim = GetScreenPosDim(
					&Entry->Basis,
					RenderGroup->WorldToCamera,
					RenderGroup->CameraToScreen
				);
				vector2 XAxis = (
					((float) Entry->Bitmap->Width) * ScreenPosDim.XAxis
				);
				vector2 YAxis = (
					((float) Entry->Bitmap->Height) * ScreenPosDim.YAxis
				);

				DrawBitmapQuickly(
					Target,
					ScreenPosDim.Pos,
					XAxis,
					YAxis,
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
	END_TIMED_BLOCK(RenderGroupToOutput);
}
