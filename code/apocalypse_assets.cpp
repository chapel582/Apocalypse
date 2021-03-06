#include "apocalypse_assets.h"

#include "apocalypse_bitmap.h"
#include "apocalypse_wav.h"
#include "apocalypse_platform.h"

// TODO: check hash of file before loading it into memory 
// CONT: Day 468 HMH?

void ResetAssets(assets* Assets)
{
	PlatformGetMutex(Assets->AvailableListLock);
	PlatformGetMutex(Assets->ArenaLock);
	
	// TODO: probably just want to complete asset jobs?
	PlatformCompleteAllJobs(Assets->JobQueue);
	for(
		int InfoIndex = 0;
		InfoIndex < ARRAY_COUNT(Assets->BitmapInfo);
		InfoIndex++
	)
	{
		asset_info* AssetInfo = &Assets->BitmapInfo[InfoIndex];
		AssetInfo->State = AssetState_Unloaded;
	}
	for(
		int InfoIndex = 0;
		InfoIndex < ARRAY_COUNT(Assets->WavInfo);
		InfoIndex++
	)
	{
		asset_info* AssetInfo = &Assets->WavInfo[InfoIndex];
		AssetInfo->State = AssetState_Unloaded;
	}
	for(
		int InfoIndex = 0;
		InfoIndex < ARRAY_COUNT(Assets->FontInfo);
		InfoIndex++
	)
	{
		asset_info* AssetInfo = &Assets->FontInfo[InfoIndex];
		AssetInfo->State = AssetState_Unloaded;
	}
	for(int FontHandle = 0; FontHandle < FontHandle_Count; FontHandle++)
	{
		for(
			int InfoIndex = 0;
			InfoIndex < ARRAY_COUNT(Assets->GlyphInfo[FontHandle]);
			InfoIndex++
		)
		{
			asset_info* AssetInfo = (
				&Assets->GlyphInfo[FontHandle][InfoIndex]
			);
			AssetInfo->State = AssetState_Unloaded;
		}
	}
	Assets->AvailableHead = NULL;

	ResetMemArena(&Assets->Arena);

	PlatformReleaseMutex(Assets->ArenaLock);
	PlatformReleaseMutex(Assets->AvailableListLock);
}

load_asset_job* GetJob(assets* Assets)
{
	load_asset_job* Job = NULL;
	PlatformGetMutex(Assets->AvailableListLock);
	if(Assets->AvailableHead)
	{
		Job = Assets->AvailableHead; 
		Assets->AvailableHead = Assets->AvailableHead->Next;
	}
	PlatformReleaseMutex(Assets->AvailableListLock);
	if(Job == NULL)
	{
		// PlatformGetMutex(Assets->ArenaLock);
		Job = PushStruct(&Assets->JobArena, load_asset_job);
		// PlatformReleaseMutex(Assets->ArenaLock);
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

loaded_bitmap* GetBitmap(assets* Assets, bitmap_handle BitmapHandle)
{
	asset_info* Info = &(Assets->BitmapInfo[BitmapHandle]);
	if(Info->State == AssetState_Loaded)
	{
		return &(Assets->Bitmaps[BitmapHandle]);
	}
	else
	{
		if(Info->State == AssetState_Unloaded)
		{		
			// NOTE: Need to start loading bitmap
			load_asset_job* Job = GetJob(Assets);

			switch(BitmapHandle)
			{
				case(BitmapHandle_TestBitmap):
				{
					strcpy_s(
						Job->FileName,
						sizeof(Job->FileName),
						"../data/tree00.bmp"
					);
					break;
				}
				case(BitmapHandle_TestBackground):
				{
					strcpy_s(
						Job->FileName,
						sizeof(Job->FileName),
						"../data/test_background.bmp"
					);
					break;
				}
				case(BitmapHandle_TestCard):
				{
					strcpy_s(
						Job->FileName,
						sizeof(Job->FileName),
						"../data/test_card.bmp"
					);
					break;
				}
				case(BitmapHandle_TestCard2):
				{
					strcpy_s(
						Job->FileName,
						sizeof(Job->FileName),
						"../data/test_card2.bmp"
					);
					break;
				}
				case(BitmapHandle_X):
				{
					strcpy_s(
						Job->FileName,
						sizeof(Job->FileName),
						"../data/x.bmp"
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
			
			InitAssetJobCommon(
				Job, Assets, &Assets->Bitmaps[BitmapHandle], Info
			);
			
			PlatformAddJob(
				Assets->JobQueue, LoadBmpJob, Job, JobPriority_Asset
			);
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

loaded_wav* GetWav(assets* Assets, wav_handle WavHandle)
{
	asset_info* Info = &(Assets->WavInfo[WavHandle]);

	loaded_wav* Result = NULL;
	if(Info->State == AssetState_Loaded)
	{
		Result = &(Assets->Wavs[WavHandle]);
	}
	else
	{
		if(Info->State == AssetState_Unloaded)
		{
			// NOTE: start loading wav
			// TODO: see if this can be parameterized with other assets
			// TODO: allocate these args with the assets' general allocator
			load_asset_job* Job = GetJob(Assets);

			switch(WavHandle)
			{
				case(WavHandle_Bloop00):
				{
					strcpy_s(
						Job->FileName,
						sizeof(Job->FileName),
						"../data/bloop_00.wav"
					);
					break;
				}
				case(WavHandle_Crack00):
				{
					strcpy_s(
						Job->FileName,
						sizeof(Job->FileName),
						"../data/crack_00.wav"
					);
					break;
				}
				case(WavHandle_TestMusic):
				{
					strcpy_s(
						Job->FileName,
						sizeof(Job->FileName),
						"../data/music_test.wav"
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

			InitAssetJobCommon(Job, Assets, &Assets->Wavs[WavHandle], Info);

			PlatformAddJob(
				Assets->JobQueue, LoadWavJob, Job, JobPriority_Asset
			);
		}
		Result = NULL;
	}

	return Result;
}

void LoadFontJob(void* VoidArgs)
{
	load_asset_job* Job = (load_asset_job*) VoidArgs;

	void* Contents;
	platform_read_file_result ReadResult = ReadEntireFile(
		Job->FileName, Job->MemoryArena, Job->ArenaLock, &Contents
	);
	if(ReadResult != PlatformReadFileResult_Success)
	{
		goto error;
	}

	loaded_font* LoadedFont = (loaded_font*) Job->Result;
	LoadedFont->PixelHeight = Job->PixelHeight;
	stbtt_fontinfo* Font = &LoadedFont->StbFont;
	stbtt_InitFont(
		Font,
		(uint8_t*) Contents,
		stbtt_GetFontOffsetForIndex((uint8_t*) Contents, 0)
	);
	// TODO: error check stbtt_InitFont return code
	
	LoadJobCommonEnd(Job);
	goto end;

error:
	// TODO: logging
end:
	return;
}

loaded_font* GetFont(assets* Assets, font_handle FontHandle)
{
	loaded_font* Result = NULL;
	asset_info* Info = &(Assets->FontInfo[FontHandle]);
	if(Assets->FontInfo[FontHandle].State == AssetState_Loaded)
	{
		Result = &Assets->Fonts[FontHandle];
	}
	else if(Assets->FontInfo[FontHandle].State == AssetState_Unloaded)
	{
		load_asset_job* Job = GetJob(Assets);
		switch(FontHandle)
		{
			case(FontHandle_TestFont):
			{
				strcpy_s(
					Job->FileName,
					sizeof(Job->FileName),
					"../data/arial.ttf"
				);
				Job->PixelHeight = 2048.0f;
				break;
			}
			case(FontHandle_DebugFont):
			{
				strcpy_s(
					Job->FileName,
					sizeof(Job->FileName),
					"../data/cour.ttf"
				);
				Job->PixelHeight = 20.0f;
				break;
			}
			default:
			{
				ASSERT(false);
				break;
			}
		}
		Info->State = AssetState_Loading;
		InitAssetJobCommon(Job, Assets, &Assets->Fonts[FontHandle], Info);
		PlatformAddJob(Assets->JobQueue, LoadFontJob, Job, JobPriority_Asset);
	}

	return Result;
}

void LoadGlyphJob(void* Data)
{
	load_asset_job* Job = (load_asset_job*) Data;
	assets* Assets = Job->Assets;
	loaded_glyph* Result = (loaded_glyph*) Job->Result; 
	uint32_t CodePoint = Job->CodePoint;
	ASSERT(Assets->FontInfo[Job->FontHandle].State == AssetState_Loaded);
	loaded_font* LoadedFont = &Assets->Fonts[Job->FontHandle];
	stbtt_fontinfo* Font = &LoadedFont->StbFont;

	int Width, Height, XOffset, YOffset;
	uint8_t* MonoBitmap = stbtt_GetCodepointBitmap(
		Font,
		0,
		stbtt_ScaleForPixelHeight(Font, LoadedFont->PixelHeight),
		CodePoint,
		&Width,
		&Height,
		&XOffset,
		&YOffset
	);

	loaded_bitmap* Bitmap = &Result->Bitmap;
	Bitmap->Width = Width;
	Bitmap->Height = Height;
	Bitmap->Pitch = Bitmap->Width * BYTES_PER_PIXEL;
	PlatformGetMutex(Job->ArenaLock);
	Bitmap->Memory = PushSize(Job->MemoryArena, Height * Bitmap->Pitch);
	PlatformReleaseMutex(Job->ArenaLock);

	uint8_t* Source = MonoBitmap;
	uint8_t* DestRow = (
		((uint8_t*) Bitmap->Memory) + (Height - 1) * Bitmap->Pitch
	);
	for(
		int32_t Y = 0;
		Y < Height;
		Y++
	)
	{
		uint32_t* Dest = (uint32_t*) DestRow;
		for(
			int32_t X = 0;
			X < Width;
			X++
		)
		{
			uint8_t Alpha = *Source++;
			*Dest++ = (
				(Alpha << 24) |
				(Alpha << 16) |
				(Alpha <<  8) |
				(Alpha <<  0)
			);
		}

		DestRow -= Bitmap->Pitch;
	}
	
	stbtt_FreeBitmap(MonoBitmap, 0);
	LoadJobCommonEnd(Job);
}

loaded_glyph* GetGlyph(
	assets* Assets, font_handle FontHandle, uint32_t CodePoint
)
{
	loaded_font* LoadedFont = GetFont(Assets, FontHandle);
	if(LoadedFont == NULL)
	{
		return NULL;
	}

	asset_info* Info = &(Assets->GlyphInfo[FontHandle][CodePoint]);
	if(Info->State == AssetState_Loaded)
	{
		return &(Assets->Glyphs[FontHandle][CodePoint]);
	}
	else
	{
		if(Info->State == AssetState_Unloaded)
		{
			// NOTE: Need to start loading bitmap
			load_asset_job* Job = GetJob(Assets);

			Info->State = AssetState_Loading;
			
			InitAssetJobCommon(
				Job, Assets, &Assets->Glyphs[FontHandle][CodePoint], Info
			);
			Job->FontHandle = FontHandle;
			Job->CodePoint = CodePoint;

			PlatformAddJob(
				Assets->JobQueue, LoadGlyphJob, Job, JobPriority_Asset
			);
		}
		return NULL;
	}
}