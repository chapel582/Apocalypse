#ifndef APOCALYPSE_ASSETS_H

typedef enum 
{
	AssetType_Bitmap
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

typedef enum 
{
	BitmapTag_TestBitmap,
	BitmapTag_TestCard,
	BitmapTag_TestBackground,
	BitmapTag_Count
} bitmap_tag_e;

struct load_bmp_job_args
{
	char FileName[256]; // TODO: Platform max path
	loaded_bitmap* Result;
	asset_info* Info;
};

struct assets
{
	platform_job_queue* JobQueue;

	load_bmp_job_args BitmapJobArgs[BitmapTag_Count]; // TODO: allocate/free these arguments dynamically
	int NextJob;
	asset_info BitmapInfo[BitmapTag_Count];
	loaded_bitmap Bitmaps[BitmapTag_Count];

	// TODO: get a general purpose memory allocator for asset data management
	// TODO: have a way to track how stale an asset is
};

loaded_bitmap* GetBitmap(assets* Assets, bitmap_tag_e AssetTag);

#define APOCALYPSE_ASSETS_H
#endif 