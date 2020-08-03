#include "apocalypse.h"
#include "apocalypse_platform.h"
#include "apocalypse_intrinsics.h"
#include "apocalypse_debug.h"
#include "apocalypse_string.h"

#include "apocalypse_render_group.h"
#include "apocalypse_render_group.cpp"

#include "apocalypse_bitmap.h"
#include "apocalypse_bitmap.cpp"

#include "apocalypse_wav.h"
#include "apocalypse_wav.cpp"

#include "apocalypse_assets.h"
#include "apocalypse_assets.cpp"

#include "apocalypse_audio.h"
#include "apocalypse_audio.cpp"

#include "apocalypse_particles.h"
#include "apocalypse_particles.cpp"

#include "apocalypse_card_game.cpp"
#include "apocalypse_main_menu.cpp"
#include "apocalypse_button.cpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

loaded_bitmap MakeEmptyBitmap(
	memory_arena* Arena, int32_t Width, int32_t Height
)
{
	void* Memory = PushSize(Arena, GetBitmapSize(Width, Height));
	return MakeEmptyBitmap(Memory, Width, Height);
}

void GameUpdateAndRender(
	game_memory* Memory,
	game_offscreen_buffer* BackBuffer,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
)
{
	TIMED_BLOCK();

	ASSERT(sizeof(game_state) <= Memory->PermanentStorageSize);
	game_state* GameState = (game_state*) Memory->PermanentStorage;
	if(!Memory->IsInitialized)
	{
#if APOCALYPSE_INTERNAL
		// NOTE: initialize debug code
		// TODO: get your own virtual memory hunk for this!
		for(int ThreadIndex = 0; ThreadIndex < MAX_THREAD_COUNT; ThreadIndex++)
		{
			GlobalDebugRecords[ThreadIndex] = {};
		}
#endif

#if APOCALYPSE_RELEASE
		// TODO: replace with bespoke random tool
		// NOTE: only init rand tools if we're in a release build
		srand(time(NULL));
#endif

		// NOTE: zero out memory at start just in case
		*GameState = {};
		// NOTE: right now, we assume everything after game state is just in the arena
		InitMemArena(
			&GameState->Arena,
			Memory->PermanentStorageSize - sizeof(game_state),
			((uint8_t*) Memory->PermanentStorage) + sizeof(game_state)
		);

		// TODO: do we want to be extra and make sure we pick up that last byte?
		memory_arena AssetArena;
		size_t TransientStorageDivision = Memory->TransientStorageSize / 4;
		InitMemArena(
			&GameState->TransientArena,
			TransientStorageDivision,
			(uint8_t*) Memory->TransientStorage
		);
		InitMemArena(
			&GameState->RenderArena,
			TransientStorageDivision,
			GetEndOfArena(&GameState->TransientArena)
		);
		InitMemArena(
			&GameState->FrameArena,
			TransientStorageDivision,
			GetEndOfArena(&GameState->RenderArena)
		);
		InitMemArena(
			&AssetArena,
			TransientStorageDivision,
			GetEndOfArena(&GameState->FrameArena)
		);

		GameState->WorldToCamera = MakeBasis(
			Vector2(BackBuffer->Width / 2.0f, BackBuffer->Height / 2.0f),
			Vector2(1.0f, 0.0f),
			Vector2(0.0f, 1.0f)
		);
		GameState->CameraToScreen = MakeBasis(
			Vector2(
				-1.0f * BackBuffer->Width / 2.0f,
				-1.0f * BackBuffer->Height / 2.0f
			),
			Vector2(1, 0),
			Vector2(0, 1)
		);

		render_group* RenderGroup = &GameState->RenderGroup;
		*RenderGroup = {};
		RenderGroup->Arena = &GameState->RenderArena;

		RenderGroup->WorldToCamera = &GameState->WorldToCamera;
		RenderGroup->CameraToScreen = &GameState->CameraToScreen;

		GameState->Time = 0;

		assets* Assets = &GameState->Assets; 
		*Assets = {}; 
		Assets->Arena = AssetArena;
		Assets->JobQueue = Memory->DefaultJobQueue;
		Assets->ArenaLock = PlatformCreateMutex();
		Assets->AvailableListLock = PlatformCreateMutex();

		Memory->IsInitialized = true;

		StartCardGame(GameState, BackBuffer);
	}

	switch(GameState->Scene)
	{
		case(SceneType_CardGame):
		{
			UpdateAndRenderCardGame(
				GameState,
				(card_game_state*) GameState->SceneState,
				BackBuffer,
				MouseEvents,
				KeyboardEvents,
				DtForFrame
			);
			break;
		}
		case(SceneType_MainMenu):
		{
			UpdateAndRenderMainMenu(
				GameState,
				(main_menu_state*) GameState->SceneState,
				BackBuffer,
				MouseEvents,
				KeyboardEvents,
				DtForFrame
			);
			break;
		}
		default:
		{
			ASSERT(false);
		}
	}
	
	// SECTION START: Render from RenderGroup
	loaded_bitmap DrawBuffer = {};
	DrawBuffer.Width = BackBuffer->Width;
	DrawBuffer.Height = BackBuffer->Height;
	DrawBuffer.Pitch = BackBuffer->Pitch;
	DrawBuffer.Memory = BackBuffer->Memory;

	RenderGroupToOutput(&GameState->RenderGroup, &DrawBuffer);
	// SECTION STOP: Render from RenderGroup

	ResetMemArena(&GameState->FrameArena);
}

void GameFillSound(game_memory* Memory, game_sound_output_buffer* SoundBuffer)
{
	ASSERT(sizeof(game_state) <= Memory->PermanentStorageSize);
	game_state* GameState = (game_state*) Memory->PermanentStorage;

	MixSounds(
		SoundBuffer,
		&GameState->FrameArena,
		&GameState->Assets,
		&GameState->PlayingSoundList
	);
}

void HandleGameDebug(game_memory* Memory, game_offscreen_buffer* BackBuffer)
{
	ASSERT(sizeof(game_state) <= Memory->PermanentStorageSize);
	game_state* GameState = (game_state*) Memory->PermanentStorage;
	if(GameState->OverlayDebugInfo)
	{
		loaded_bitmap DrawBuffer = {};
		DrawBuffer.Width = BackBuffer->Width;
		DrawBuffer.Height = BackBuffer->Height;
		DrawBuffer.Pitch = BackBuffer->Pitch;
		DrawBuffer.Memory = BackBuffer->Memory;

		uint32_t MaxDebugInfoStringSize = 2048; 
		char* DebugInfoString = PushArray(
			&GameState->FrameArena, MaxDebugInfoStringSize, char
		);
		string_appender StringAppender = MakeStringAppender(
			DebugInfoString, MaxDebugInfoStringSize
		);
		for(
			uint32_t ThreadIndex = 0;
			ThreadIndex < MAX_THREAD_COUNT;
			ThreadIndex++
		)
		{
			bool ThreadInUse = false;
			thread_debug_records* ThreadDebugRecords = (
				&GlobalDebugRecords[ThreadIndex]
			);
			for(
				uint32_t DebugRecordIndex = 0; 
				DebugRecordIndex < MAX_DEBUG_RECORDS_PER_THREAD;
				DebugRecordIndex++
			)
			{
				debug_record* DebugRecord = (
					&ThreadDebugRecords->Records[DebugRecordIndex]
				);
				if(DebugRecord->HitCount)
				{
					if(!ThreadInUse)
					{
						AppendToString(
							&StringAppender, "Thread %d\n", ThreadIndex
						);
						ThreadInUse = true;	
					}
					AppendToString(
						&StringAppender,
						"%s:%d %I64ucy %uh %I64ucy/h\n",
						DebugRecord->FunctionName,
						DebugRecord->LineNumber,
						DebugRecord->CycleCount,
						DebugRecord->HitCount,
						DebugRecord->CycleCount / DebugRecord->HitCount
					);
					DebugRecord->CycleCount = 0;
					DebugRecord->HitCount = 0;
				}
			}
		}
		TerminateString(&StringAppender);

		PushText(
			&GameState->RenderGroup,
			&GameState->Assets,
			FontHandle_TestFont,
			DebugInfoString,
			MaxDebugInfoStringSize,
			20.0f,
			Vector2(
				0.0f, 
				BackBuffer->Height - 30.0f
			),
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			&GameState->FrameArena
		);

		RenderGroupToOutput(&GameState->RenderGroup, &DrawBuffer);	
		ResetMemArena(&GameState->FrameArena);
	}
}