#ifndef APOCALYPSE_ASSETS_H

#include "apocalypse_platform.h"
#include "apocalypse_bitmap.h"
#include "apocalypse_wav.h"

// TODO: for standardness, will probably want to rename tags to handles 
typedef enum 
{
	AssetType_Bitmap,
	AssetType_Wav
} asset_type_e;

typedef enum
{
	AssetState_Unloaded,
	AssetState_Loading,
	AssetState_Loaded
} asset_state_e;

struct asset_info
{
	asset_state_e State;
};

struct load_asset_job;
struct load_asset_job
{
	char FileName[256]; // TODO: Platform max path
	void* Result;
	asset_info* Info;
	memory_arena* MemoryArena;
	platform_mutex_handle* ArenaLock;
	platform_mutex_handle* AvailableListLock;
	load_asset_job* Next;
	load_asset_job** AvailableHead;
};

typedef enum 
{
	BitmapTag_TestBitmap,
	BitmapTag_TestCard,
	BitmapTag_TestBackground,
	BitmapTag_Count
} bitmap_tag_e;

typedef enum
{
	WavTag_Bloop00,
	WavTag_Crack00,
	WavTag_TestMusic,
	WavTag_Count
} wav_tag_e;

struct assets
{
	memory_arena Arena;
	platform_mutex_handle* ArenaLock;
	
	platform_job_queue* JobQueue;

	platform_mutex_handle* AvailableListLock;
	load_asset_job* AvailableHead;

	asset_info BitmapInfo[BitmapTag_Count];
	loaded_bitmap Bitmaps[BitmapTag_Count];

	asset_info WavInfo[WavTag_Count];
	loaded_wav Wavs[WavTag_Count];

	// TODO: get a general purpose memory allocator for asset data management
	// TODO: have a way to track how stale an asset is
};

loaded_bitmap* GetBitmap(assets* Assets, bitmap_tag_e AssetTag);

#define APOCALYPSE_ASSETS_H
#endif 