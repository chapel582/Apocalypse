#include "apocalypse_assets.h"

#include "apocalypse_bitmap.h"

void LoadBmpJob(void* Data)
{
	load_bmp_job* Args = (load_bmp_job*) Data;
	*(Args->Result) = DEBUGLoadBmp(&Args->FileName[0]);
	Args->Info->State = AssetState_Loaded;
}

void StartLoadingBitmap(assets* Assets, bitmap_tag_e Tag)
{
	asset_info* Info = &(Assets->BitmapInfo[Tag]);
	load_bmp_job* Args = &(Assets->BitmapJobArgs[Assets->NextJob++]);
	switch(Tag)
	{
		case(BitmapTag_TestBitmap):
		{
			strcpy_s(
				Args->FileName,
				sizeof(Args->FileName),
				"../data/test/tree00.bmp"
			);
			break;
		}
		case(BitmapTag_TestCard):
		{
			strcpy_s(
				Args->FileName,
				sizeof(Args->FileName),
				"../data/test/test_card.bmp"
			);
			break;
		}
		case(BitmapTag_TestBackground):
		{
			strcpy_s(
				Args->FileName,
				sizeof(Args->FileName),
				"../data/test/test_background.bmp"
			);
			break;
		}
		default:
		{
			ASSERT(false);
			break;
		}
	}
	Args->Result = &(Assets->Bitmaps[Tag]);
	Args->Info = Info;
	Info->State = AssetState_Loading;
	PlatformAddJob(Assets->JobQueue, LoadBmpJob, Args);
}

loaded_bitmap* GetBitmap(assets* Assets, bitmap_tag_e AssetTag)
{
	asset_info* Info = &(Assets->BitmapInfo[AssetTag]);
	if(Info->State == AssetState_Unloaded)
	{
		StartLoadingBitmap(Assets, AssetTag);
		return NULL;
	}
	else
	{
		return &(Assets->Bitmaps[AssetTag]);
	}
}