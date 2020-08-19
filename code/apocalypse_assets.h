#ifndef APOCALYPSE_ASSETS_H

#include "apocalypse_platform.h"
#include "apocalypse_bitmap.h"
#include "apocalypse_wav.h"

#include "stb_truetype.h"

// TODO: for standardness, will probably want to rename tags to handles 
typedef enum 
{
	AssetType_Bitmap,
	AssetType_Wav
} asset_type;

typedef enum
{
	AssetState_Unloaded,
	AssetState_Loading,
	AssetState_Loaded
} asset_state;

struct asset_info
{
	asset_state State;
};

typedef enum 
{
	BitmapHandle_TestBitmap,
	BitmapHandle_TestBackground,
	BitmapHandle_TestCard,
	BitmapHandle_TestCard2,
	BitmapHandle_Count
} bitmap_handle;

typedef enum
{
	WavHandle_Bloop00,
	WavHandle_Crack00,
	WavHandle_TestMusic,
	WavHandle_Count
} wav_handle;

typedef enum
{
	FontHandle_TestFont,
	FontHandle_DebugFont,
	FontHandle_Count
} font_handle;

struct assets;
struct load_asset_job;
struct load_asset_job
{
	char FileName[PLATFORM_MAX_PATH];
	assets* Assets;
	void* Result;
	asset_info* Info;
	memory_arena* MemoryArena;
	platform_mutex_handle* ArenaLock;
	platform_mutex_handle* AvailableListLock;
	load_asset_job* Next;
	load_asset_job** AvailableHead;

	// NOTE: if other args start getting too big, we can have each asset type 
	// CONT: have its own job structure and list
	
	// NOTE: for glyph loading
	font_handle FontHandle;
	uint32_t CodePoint;
	float PixelHeight;
};

struct loaded_font
{
	float PixelHeight;
	stbtt_fontinfo StbFont;
};
struct loaded_glyph
{
	loaded_bitmap Bitmap;
};

struct assets
{
	memory_arena Arena;
	platform_mutex_handle* ArenaLock;
	
	platform_job_queue* JobQueue;

	platform_mutex_handle* AvailableListLock;
	load_asset_job* AvailableHead;

	asset_info BitmapInfo[BitmapHandle_Count];
	loaded_bitmap Bitmaps[BitmapHandle_Count];

	asset_info WavInfo[WavHandle_Count];
	loaded_wav Wavs[WavHandle_Count];

	asset_info FontInfo[FontHandle_Count];
	loaded_font Fonts[FontHandle_Count];
	// TODO: add a BST for sparse glyph support. Use BST to determine if glyph
	// CONT: is already loaded or not
	asset_info GlyphInfo[FontHandle_Count][256];
	loaded_glyph Glyphs[FontHandle_Count][256];
};

inline void InitAssetJobCommon(
	load_asset_job* Job,
	assets* Assets,
	void* Result,
	asset_info* Info
)
{
	Job->Assets = Assets;
	Job->Result = Result;
	Job->Info = Info;
	Job->MemoryArena = &Assets->Arena;
	Job->ArenaLock = Assets->ArenaLock;
	Job->AvailableListLock = Assets->AvailableListLock;
	Job->Next = NULL;
	Job->AvailableHead = &Assets->AvailableHead;
}

loaded_bitmap* GetBitmap(assets* Assets, bitmap_handle BitmapHandle);
loaded_glyph* GetGlyph(
	assets* Assets, font_handle FontHandle, uint32_t CodePoint
);
loaded_font* GetFont(assets* Assets, font_handle FontHandle);

#define APOCALYPSE_ASSETS_H
#endif 