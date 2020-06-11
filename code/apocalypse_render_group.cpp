#include "apocalypse_render_group.h"

#include "apocalypse_math.h"
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
	loaded_bitmap* Bitmap,
	basis Basis
)
{
	// TODO: we might want a way to push a bitmap while only designating its 
	// CONT: dimentions in world space
	// TODO: may want a way to push a centered bitmap
	// NOTE: basis offset should be the top left of the unrotated texture
	render_entry_bitmap* Entry = PushStruct(Group->Arena, render_entry_bitmap);
	Group->LastEntry = (uint8_t*) Entry;
	Entry->Header.Type = EntryType_Bitmap;
	Entry->Basis = Basis;
	Entry->Bitmap = Bitmap;
}

inline void PushBitmapCentered(
	render_group* Group,
	loaded_bitmap* Bitmap,
	vector2 Center,
	vector2 XAxis,
	vector2 YAxis
)
{
	// NOTE: XAxis and YAxis describes the rotation and scaling of the bitmap
	// CONT: but Y is negated for them, so rotations should be thought of as CW,
	// CONT: not counter clockwise (e.g. XAxis = {0.707, 0.707} is a 45 degree)
	// CONT: rotation *clockwise* b/c down is positive for Y in screen space

	// NOTE: We need to offset the center of world space by an amount in screen 
	// CONT: space in order to keep the bitmap centered
	vector2 ScreenXAxis = XAxis;
	ScreenXAxis.Y *= -1.0f;
	vector2 ScreenYAxis = YAxis;
	ScreenYAxis.Y *= -1.0f;

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
		)
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
	vector2 Origin,
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
		(RoundFloat32ToInt32(Red * 0xFF) << 16) |
		(RoundFloat32ToInt32(Green * 0xFF) << 8) |
		RoundFloat32ToInt32(Blue * 0xFF)
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
				Origin,
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

void DrawBitmapSlowly(
	loaded_bitmap* Buffer, 
	vector2 Origin,
	vector2 XAxis,
	vector2 YAxis,
	loaded_bitmap* Texture
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

	uint32_t Color = 0xFFFFFFFF;

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
				Origin,
				XAxis,
				YAxis,
				XAxisPerp,
				YAxisPerp
			);
			if(InRotatedQuad)
			{
				// NOTE: U and V are determined by normalizing the projection of
				// CONT: the point to each axis
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
				
				uint8_t* TexelPtr = (
					((uint8_t*) Texture->Memory) + 
					iY * Texture->Pitch + 
					iX * sizeof(uint32_t)
				);

				// NOTE: lerp based on texels below above and to the right
				uint32_t TexelPtrA = *(uint32_t*) (TexelPtr);
				uint32_t TexelPtrB = *(uint32_t*) (TexelPtr + sizeof(uint32_t));
				uint32_t TexelPtrC = *(uint32_t*) (TexelPtr + Texture->Pitch);
				uint32_t TexelPtrD = *(uint32_t *) (
					TexelPtr + Texture->Pitch + sizeof(uint32_t)
				);

				// TODO: Color.a!!
				vector4 TexelA = {
					(float)((TexelPtrA >> 16) & 0xFF),
					(float)((TexelPtrA >> 8) & 0xFF),
					(float)((TexelPtrA >> 0) & 0xFF),
					(float)((TexelPtrA >> 24) & 0xFF)
				};
				vector4 TexelB = {
					(float)((TexelPtrB >> 16) & 0xFF),
					(float)((TexelPtrB >> 8) & 0xFF),
					(float)((TexelPtrB >> 0) & 0xFF),
					(float)((TexelPtrB >> 24) & 0xFF)
				};
				vector4 TexelC = {
					(float)((TexelPtrC >> 16) & 0xFF),
					(float)((TexelPtrC >> 8) & 0xFF),
					(float)((TexelPtrC >> 0) & 0xFF),
					(float)((TexelPtrC >> 24) & 0xFF)
				};
				vector4 TexelD = {
					(float)((TexelPtrD >> 16) & 0xFF),
					(float)((TexelPtrD >> 8) & 0xFF),
					(float)((TexelPtrD >> 0) & 0xFF),
					(float)((TexelPtrD >> 24) & 0xFF)
				};

				// NOTE: Lerp to get the colors between
				vector4 Texel = Lerp(
					Lerp(TexelA, fX, TexelB),
					fY,
					Lerp(TexelC, fX, TexelD)
				);
				
				// NOTE: source colors and alpha
				float SA = Texel.A;
				float SR = Texel.R;
				float SG = Texel.G;
				float SB = Texel.B;

				// NOTE: normalize alpha
				float A = SA / 255.0f;

				float DR = (float) (((*Pixel >> 16) & 0xFF));
				float DG = (float) (((*Pixel >> 8) & 0xFF));
				float DB = (float) (((*Pixel) & 0xFF));

				float R = ((1.0f - A) * DR) + (A * SR);
				float G = ((1.0f - A) * DG) + (A * SG);
				float B = ((1.0f - A) * DB) + (A * SB);

				*Pixel = (
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

void Clear(
	loaded_bitmap* Buffer, 
	float Red,
	float Green, 
	float Blue
)
{
	DrawRectangle(
		Buffer,
		MakeRectangle(
			Vector2(0.0f, 0.0f),
			Vector2((float) Buffer->Width, (float) Buffer->Height)
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
					Target, Entry->Color.R, Entry->Color.G, Entry->Color.B
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
					Entry->Bitmap
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
