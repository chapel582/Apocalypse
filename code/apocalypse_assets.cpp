#include "apocalypse_assets.h"

#include "apocalypse_bitmap.h"
#include "apocalypse_wav.h"
#include "apocalypse_platform.h"

load_asset_job* GetJob(assets* Assets)
{
	load_asset_job* Job = NULL;
	if(Assets->AvailableHead)
	{
		PlatformGetMutex(Assets->AvailableListLock);
		Job = Assets->AvailableHead; 
		Assets->AvailableHead = Assets->AvailableHead->Next;
		PlatformReleaseMutex(Assets->AvailableListLock);
	}
	else
	{
		PlatformGetMutex(Assets->ArenaLock);
		Job = PushStruct(&Assets->Arena, load_asset_job);
		PlatformReleaseMutex(Assets->ArenaLock);
	}
	return Job;
}

void LoadJobCommonEnd(load_asset_job* Job)
{
	Job->Info->State = AssetState_Loaded;
	PlatformGetMutex(Job->AvailableListLock);
	Job->Next = *Job->AvailableHead;
	*(Job->AvailableHead) = Job;
	PlatformReleaseMutex(Job->AvailableListLock);
}

void LoadBmpJob(void* Data)
{
	load_asset_job* Job = (load_asset_job*) Data;
	loaded_bitmap* Result = (loaded_bitmap*) Job->Result;
	*Result = LoadBmp(&Job->FileName[0], Job->MemoryArena, Job->ArenaLock);
	LoadJobCommonEnd(Job);
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
			load_asset_job* Args = GetJob(Assets);

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
			Info->State = AssetState_Loading;
			
			Args->Result = &(Assets->Bitmaps[Tag]);
			Args->Info = Info;
			Args->MemoryArena = &Assets->Arena;
			Args->ArenaLock = Assets->ArenaLock;
			Args->AvailableListLock = Assets->AvailableListLock;
			Args->Next = NULL;
			Args->AvailableHead = &Assets->AvailableHead;
			PlatformAddJob(Assets->JobQueue, LoadBmpJob, Args);
		}
		return NULL;
	}
}

void LoadWavJob(void* Data)
{
	load_asset_job* Job = (load_asset_job*) Data;
	loaded_wav* Result = (loaded_wav*) Job->Result;
	*Result = LoadWav(&Job->FileName[0], Job->MemoryArena, Job->ArenaLock);
	LoadJobCommonEnd(Job);
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
			load_asset_job* Args = GetJob(Assets);

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
			Info->State = AssetState_Loading;

			Args->Result = &(Assets->Wavs[Tag]);
			Args->Info = Info;
			Args->MemoryArena = &Assets->Arena;
			Args->ArenaLock = Assets->ArenaLock;
			Args->AvailableListLock = Assets->AvailableListLock;
			Args->Next = NULL;
			Args->AvailableHead = &Assets->AvailableHead;
			PlatformAddJob(Assets->JobQueue, LoadWavJob, Args);
		}
		Result = NULL;
	}

	return Result;
}