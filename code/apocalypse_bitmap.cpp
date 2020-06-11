#include "apocalypse_bitmap.h"
#include "apocalypse_platform.h"

#pragma pack(push, 1)
struct bitmap_header
{
	uint16_t FileType;
    uint32_t FileSize;
    uint16_t Reserved1;
    uint16_t Reserved2;
    uint32_t BitmapOffset;
    uint32_t BitmapSize;
    int32_t Width;
    int32_t Height;
    uint16_t Planes;
    uint16_t BitsPerPixel;
    uint32_t Compression;
    uint32_t SizeOfBitmap;
    int32_t HorzResolution;
    int32_t VertResolution;
    uint32_t ColorsUsed;
    uint32_t ColorsImportant;

    uint32_t RedMask;
    uint32_t GreenMask;
    uint32_t BlueMask;
};
#pragma pack(pop)

loaded_bitmap DEBUGLoadBmp(thread_context* Thread, char* FileName)
{
	// NOTE: this is not complete BMP loading code. Can't handle negative height
	// CONT: or compression

	loaded_bitmap Result = {};
	debug_read_file_result ReadResult = DEBUGPlatformReadEntireFile(
		Thread, FileName
	);
	if(ReadResult.ContentsSize == 0)
	{
		goto end;
	}

	bitmap_header* Header = (bitmap_header*) ReadResult.Contents;
	uint32_t* Pixels = (uint32_t*) (
		(uint8_t*) ReadResult.Contents + Header->BitmapOffset
	);
	Result.Memory = Pixels;
	Result.Width = Header->Width;
	Result.Height = Header->Height;

	ASSERT(Header->Compression == 3);

	// NOTE: we use the masks to figure out how to transform our pixels into 
	// CONT: ARGB format
	uint32_t RedMask = Header->RedMask;
	uint32_t GreenMask = Header->GreenMask;
	uint32_t BlueMask = Header->BlueMask;
	// NOTE: AlphaMask is the bits that aren't in the color channels
	uint32_t AlphaMask = ~(RedMask | GreenMask | BlueMask);

	// NOTE: need to figure out how much to shift to transform stuff
	bit_scan_result RedShift = FindLeastSignificantSetBit(RedMask);
	bit_scan_result GreenShift = FindLeastSignificantSetBit(GreenMask);
	bit_scan_result BlueShift = FindLeastSignificantSetBit(BlueMask);
	bit_scan_result AlphaShift = FindLeastSignificantSetBit(AlphaMask);

	ASSERT(RedShift.Found);
	ASSERT(GreenShift.Found);
	ASSERT(BlueShift.Found);
	ASSERT(AlphaShift.Found);

	uint32_t* SourceDest = Pixels;
	for(int32_t Y = 0; Y < Header->Height; Y++)
	{
		for(int32_t X = 0; X < Header->Width; X++)
		{
			uint32_t Original = *SourceDest;
			*SourceDest++ = (
				(((Original >> AlphaShift.Index) & 0xFF) << 24) |
				(((Original >> RedShift.Index) & 0xFF) << 16) |
				(((Original >> GreenShift.Index) & 0xFF) << 8) |
				((Original >> BlueShift.Index) & 0xFF)
			);
		}
	}

end:
	return Result;
}