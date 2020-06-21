#include "apocalypse_render_group.h"

#include "apocalypse_math.h"
#include "apocalypse_rectangle.h"
#include "apocalypse_vector.h"

vector2 WorldToBasis(basis* Basis, vector2 Vector)
{
	vector2 Result = {};
	vector2 Position = Vector - Basis->Offset;
	Result = (
		Position.X * Basis->Axis1 + 
		Position.Y * Basis->Axis2 
	);
	return Result;
}

vector2 BasisToWorld(basis* Basis, vector2 Vector)
{
	vector2 Result = {};
	Result = (
		Vector.X * Basis->MatrixColumn1 + 
		Vector.Y * Basis->MatrixColumn2
	);
	Result += Basis->Offset;
	return Result;
}

vector2 WorldToBasisScale(basis* Basis, vector2 Vector)
{
	vector2 Result = {};
	Result = (
		Vector.X * Basis->Axis1 + 
		Vector.Y * Basis->Axis2
	);
	Result = Abs(Result);
	return Result;
}

vector2 BasisToWorldScale(basis* Basis, vector2 Vector)
{
	vector2 Result = {};
	Result = (
		Vector.X * Basis->MatrixColumn1 + 
		Vector.Y * Basis->MatrixColumn2
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

inline void PushBitmapCentered(
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
	// NOTE: XAxis and YAxis describes the rotation and scaling of the bitmap
	// CONT: but Y is negated for them, so rotations should be thought of as CW,
	// CONT: not counter clockwise (e.g. XAxis = {0.707, 0.707} is a 45 degree)
	// CONT: rotation *clockwise* b/c down is positive for Y in screen space

	// NOTE: We need to offset the center of world space by an amount in screen 
	// CONT: space in order to keep the bitmap centered
	vector2 ScreenXAxis = XAxis;
	// ScreenXAxis.Y *= -1.0f;
	vector2 ScreenYAxis = YAxis;
	// ScreenYAxis.Y *= -1.0f;

	vector2 BitmapDim = WorldToBasisScale(
		Group->WorldScreenBasis,
		Vector2(Bitmap->Width, Bitmap->Height)
	);

	PushBitmap(
		Group,
		Bitmap,
		MakeBasis(
			(
				Center - 
				0.5f * ScreenXAxis * BitmapDim.X - 
				0.5f * ScreenYAxis * BitmapDim.Y
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

inline vector4 Srgb255ToLinear1(vector4 C)
{
	vector4 Result;

	float Inv255 = 1.0f / 255.0f;

	Result.R = (float) pow(Inv255 * C.R, 2);
	Result.G = (float) pow(Inv255 * C.G, 2);
	Result.B = (float) pow(Inv255 * C.B, 2);
	Result.A = Inv255 * C.A;

	return Result;
}

inline vector4 Linear1ToSrgb255(vector4 C)
{
	vector4 Result;

	Result.R = 255.0f * sqrtf(C.R);
	Result.G = 255.0f * sqrtf(C.G);
	Result.B = 255.0f * sqrtf(C.B);
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

inline vector4 BilinearBlend(
	loaded_bitmap* Texture, int32_t X, int32_t Y, float LerpX, float LerpY
)
{
	// NOTE: lerp based on texels below above and to the right
	bilinear_sample Sample = BilinearSample(Texture, X, Y);

	// TODO: Color.a!!
	vector4 TexelA = Unpack4x8(Sample.A);
	vector4 TexelB = Unpack4x8(Sample.B);
	vector4 TexelC = Unpack4x8(Sample.C);
	vector4 TexelD = Unpack4x8(Sample.D);

	// NOTE: normalized texel colors
	TexelA = Srgb255ToLinear1(TexelA);
	TexelB = Srgb255ToLinear1(TexelB);
	TexelC = Srgb255ToLinear1(TexelC);
	TexelD = Srgb255ToLinear1(TexelD);

	// NOTE: Lerp to estimate the new value
	vector4 Texel = Lerp(
		Lerp(TexelA, LerpX, TexelB),
		LerpY,
		Lerp(TexelC, LerpX, TexelD)
	);
	return Texel;
}

inline vector3 SampleEnvironmentMap(
	vector2 ScreenSpaceUv,
	vector3 SampleDirection,
	float Roughness,
	environment_map* Map,
	float DistanceFromMapInZ
)
{
	/* NOTE:
		ScreenSpaceUV tells us where the ray is being cast _from_ in
		normalized screen coordinates.

		SampleDirection tells us what direction the cast is going -
		it does not have to be normalized.

		Roughness says which LODs of Map we sample from.

		DistanceFromMapInZ says how far the map is from the sample point in Z, 
		given in meters.
    */

	uint32_t LodIndex = (
		(uint32_t) ((Roughness * ((float) (ARRAY_COUNT(Map->Lod)) - 1) + 0.5f))
	);
	ASSERT(LodIndex < ARRAY_COUNT(Map->Lod));

	loaded_bitmap* Lod = &Map->Lod[LodIndex];

	// TODO: do a more formal conversion here
	float ScreenSpacePerUnit = 0.01f;
	float DistanceFromMapInScreenSpace = (
		ScreenSpacePerUnit * DistanceFromMapInZ
	);
	float Constant = DistanceFromMapInScreenSpace / SampleDirection.Y;
	vector2 Offset = Constant * Vector2(SampleDirection.X, SampleDirection.Z);
	vector2 Uv = ScreenSpaceUv + Offset;

	Uv.X = Clamp01(Uv.X);
	Uv.Y = Clamp01(Uv.Y);

	float tX = Uv.X * ((float) (Lod->Width - 2));
	float tY = Uv.Y * ((float) (Lod->Height - 2));
	
	int32_t X = (int32_t) tX;
	int32_t Y = (int32_t) tY;
	ASSERT((X >= 0) && (X < Lod->Width));
	ASSERT((Y >= 0) && (Y < Lod->Height));

	float fX = tX - (float) X;
	float fY = tY - (float) Y;

	vector3 Result = BilinearBlend(Lod, X, Y, fX, fY).Xyz;
	return Result;
}


inline vector4 UnscaleAndBiasNormal(vector4 Normal)
{
	vector4 Result;

	float Inv255 = 1.0f / 255.0f;

	Result.X = -1.0f + 2.0f * (Inv255 * Normal.X);
	Result.Y = -1.0f + 2.0f * (Inv255 * Normal.Y);
	Result.Z = -1.0f + 2.0f*(Inv255 * Normal.Z);

	Result.W = Inv255 * Normal.W;

	return(Result);
}

void DrawBitmapSlowly(
	loaded_bitmap* Buffer, 
	vector2 Origin,
	vector2 XAxis,
	vector2 YAxis,
	loaded_bitmap* Texture,
	vector4 Color,
	loaded_bitmap* NormalMap,
	environment_map* Top,
	environment_map* Middle,
	environment_map* Bottom
)
{
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
				// NOTE: U and V are determined by normalizing the projection of
				// CONT: the point to each axis
				vector2 ScreenSpaceUv = {
					InvWidthMax * (float) X, InvHeightMax * (float) Y
				};
				float U = (
					InvXAxisLengthSq * Inner(PointOffsetFromOrigin, XAxis)
				);
				float V = (
					InvYAxisLengthSq * Inner(PointOffsetFromOrigin, YAxis)
				);

				// TODO: clamping
				ASSERT((U >= 0.0f) && (U <= 1.0f));
				ASSERT((V >= 0.0f) && (V <= 1.0f));
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

				vector4 Texel = BilinearBlend(Texture, iX, iY, fX, fY);
				float NormalTexelAlpha = Texel.A;

				if(NormalMap)
				{
					// NOTE: the lighting code does not completely work, 
					// CONT: but does work for 
					// CONT: texture maps of CONSTANT color
					// CONT: it probably needs to be completely redone if you 
					// CONT: have any patterns on the light
					bilinear_sample NormalSample = BilinearSample(
						NormalMap, iX, iY
					);

					vector4 NormalA = Unpack4x8(NormalSample.A);
					vector4 NormalB = Unpack4x8(NormalSample.B);
					vector4 NormalC = Unpack4x8(NormalSample.C);
					vector4 NormalD = Unpack4x8(NormalSample.D);

					vector4 Normal = Lerp(
						Lerp(NormalA, fX, NormalB),
						fY,
						Lerp(NormalC, fX, NormalD)
					);

					Normal = UnscaleAndBiasNormal(Normal);
					// TODO: Do we really need to do this?

					Normal.Xy = Normal.X * NxAxis + Normal.Y * NyAxis;
					Normal.Z = NzScale;
					Normal.Xyz = Normalize(Normal.Xyz);

					// NOTE: the vector to the "eye" is assumed to be {0, 0, 1}
					// CONT: this is reasonable since Y determines which env 
					// CONT: map to use and x,z determine how to look up into
					// CONT: the map
					// CONT: therefore, the bounce direction equation is a 
					// CONT: simplified version of the reflection across the 
					// CONT: normal: -v + 2 * Inner(v, Normal) * Normal 
					vector3 BounceDirection = 2.0f * Normal.Z * Normal.Xyz;
					BounceDirection.Z -= 1.0f;

					// TODO: Eventually we need to support two mappings,
					// CONT: one for top-down view (which we don't do now) and 
					// CONT: one for sideways, which is what's happening here.
					BounceDirection.Z = -1 * BounceDirection.Z;

					// NOTE: Y element determines which map to use for lighting
					environment_map* FarMap = 0;
					float DistanceFromMapInZ = 2.0f;
					float TEnvMap = BounceDirection.Y;
					float TFarMap = 0.0f;
					if(TEnvMap < -0.5f)
					{
						FarMap = Bottom;
						TFarMap = -1.0f - 2.0f * TEnvMap;
						DistanceFromMapInZ = -1 * DistanceFromMapInZ;
					}
					else if(TEnvMap > 0.5f)
					{
						FarMap = Top;
						TFarMap = 2.0f * (TEnvMap - 0.5f);
					}

					vector3 LightColor = {0, 0, 0}; 
					// SampleEnvironmentMap(ScreenSpaceUV, Normal.xyz, Normal.w, Middle);
					if(FarMap)
					{
						vector3 FarMapColor = SampleEnvironmentMap(
							ScreenSpaceUv,
							BounceDirection,
							Normal.W,
							FarMap,
							DistanceFromMapInZ
						);
						LightColor = Lerp(LightColor, TFarMap, FarMapColor);
					}

					// TODO: ? Actually do a lighting model computation here
					// CONT: in order to determine how much to blend
					Texel.Rgb = Lerp(Texel.Rgb, NormalTexelAlpha, LightColor);
				}

				Texel = Hadamard(Texel, Color);
				// TODO: what's the point of the clamping?
				Texel.R = Clamp01(Texel.R);
				Texel.G = Clamp01(Texel.G);
				Texel.B = Clamp01(Texel.B); 
				
				// NOTE: source colors and alpha
				vector4 IntTexel = Linear1ToSrgb255(Texel);
				float SR = IntTexel.R;
				float SG = IntTexel.G;
				float SB = IntTexel.B;

				float DR = (float) (((*Pixel >> 16) & 0xFF));
				float DG = (float) (((*Pixel >> 8) & 0xFF));
				float DB = (float) (((*Pixel) & 0xFF));

				float R = (
					((1.0f - NormalTexelAlpha) * DR) + (NormalTexelAlpha * SR)
				);
				float G = (
					((1.0f - NormalTexelAlpha) * DG) + (NormalTexelAlpha * SG)
				);
				float B = (
					((1.0f - NormalTexelAlpha) * DB) + (NormalTexelAlpha * SB)
				);

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
	render_group* RenderGroup, loaded_bitmap* Target
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
				vector2 OriginScreenPos = GetScreenPos(
					&Entry->Basis,
					RenderGroup->WorldScreenBasis,
					Vector2(0, 0),
					RenderGroup->CameraPos
				);
				vector2 Dim = WorldToBasisScale(
					RenderGroup->WorldScreenBasis, Entry->Dim
				);
				vector2 Axis1 = Dim.X * Entry->Basis.Axis1;
				vector2 Axis2 = Dim.Y * Entry->Basis.Axis2;

				DrawRectangleSlowly(
					Target,
					OriginScreenPos,
					Axis1,
					Axis2,
					Entry->Color
				);
				CurrentAddress += sizeof(*Entry);
				break;
			}
			case(EntryType_Bitmap):
			{
				render_entry_bitmap* Entry = (render_entry_bitmap*) Header;

				vector2 OriginScreenPos = GetScreenPos(
					&Entry->Basis,
					RenderGroup->WorldScreenBasis,
					Vector2(0, 0),
					RenderGroup->CameraPos
				);

				vector2 Axis1 = (
					((float) Entry->Bitmap->Width) * Entry->Basis.Axis1
				);
				vector2 Axis2 = (
					((float) Entry->Bitmap->Height) * Entry->Basis.Axis2
				);

				DrawBitmapSlowly(
					Target,
					OriginScreenPos,
					Axis1,
					Axis2,
					Entry->Bitmap,
					Entry->Color,
					Entry->NormalMap,
					Entry->Top,
					Entry->Middle,
					Entry->Bottom
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
