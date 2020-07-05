#include "apocalypse_assets.h"

#include "apocalypse_bitmap.h"

void LoadBmpJob(void* Data)
{
	load_bmp_job* Args = (load_bmp_job*) Data;
	*(Args->Result) = LoadBmp(&Args->FileName[0], Args->MemoryArena);
	Args->Info->State = AssetState_Loaded;
}

loaded_bitmap* GetBitmap(assets* Assets, bitmap_tag_e Tag)
{
	asset_info* Info = &(Assets->BitmapInfo[Tag]);
	if(Info->State == AssetState_Loaded)
	{
		return &(Assets->Bitmaps[Tag]);
	}
	else
	{
		if(Info->State == AssetState_Unloaded)
		{		
			// NOTE: Need to start loading bitmap
			// TODO: see if there's a way to paramaterize this for loading 
			// CONT: sounds
			load_bmp_job* Args = &(Assets->BitmapJobs[Assets->NextJob++]);
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
			Args->MemoryArena = Assets->Arena;
			PlatformAddJob(Assets->JobQueue, LoadBmpJob, Args);
		}
		return NULL;
	}
}

void LoadWavJob(void* Data)
{
	load_wav_job* Args = (load_wav_job*) Data;
	*(Args->Result) = LoadWav(&Args->FileName[0], Args->MemoryArena);
	Args->Info->State = AssetState_Loaded;
}

loaded_wav* GetWav(assets* Assets, wav_tag_e Tag)
{
	asset_info* Info = &(Assets->WavInfo[Tag]);

	loaded_wav* Result = NULL;
	if(Info->State == AssetState_Loaded)
	{
		Result = &(Assets->Wavs[Tag]);
	}
	else
	{
		if(Info->State == AssetState_Unloaded)
		{
			// NOTE: start loading wav
			// TODO: see if this can be parameterized with other assets
			// TODO: allocate these args with the assets' general allocator
			load_wav_job* Args = &(Assets->WavJobs[Assets->WavNextJob++]);
			switch(Tag)
			{
				case(WavTag_Bloop00):
				{
					strcpy_s(
						Args->FileName,
						sizeof(Args->FileName),
						"../data/test/bloop_00.wav"
					);
					break;
				}
				case(WavTag_Crack00):
				{
					strcpy_s(
						Args->FileName,
						sizeof(Args->FileName),
						"../data/test/crack_00.wav"
					);
					break;
				}
				case(WavTag_TestMusic):
				{
					strcpy_s(
						Args->FileName,
						sizeof(Args->FileName),
						"../data/test/music_test.wav"
					);
					break;
				}
				default:
				{
					ASSERT(false);
					break;
				}
			}
			Args->Result = &(Assets->Wavs[Tag]);
			Args->Info = Info;
			Info->State = AssetState_Loading;
			Args->MemoryArena = Assets->Arena;
			PlatformAddJob(Assets->JobQueue, LoadWavJob, Args);
		}
		Result = NULL;
	}

	return Result;
}