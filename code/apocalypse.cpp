#include "apocalypse.h"
#include "apocalypse_platform.h"
#include "apocalypse_intrinsics.h"
#include "apocalypse_debug.h"
#include "apocalypse_string.h"
#include "apocalypse_render_group.h"
#include "apocalypse_bitmap.h"
#include "apocalypse_wav.h"
#include "apocalypse_assets.h"
#include "apocalypse_audio.h"
#include "apocalypse_particles.h"
#include "apocalypse_opengl.h"

// TODO: move the source includes above the headers
#include "apocalypse_socket.cpp"
#include "apocalypse_binary_heap.cpp"
#include "apocalypse_host_game.cpp"
#include "apocalypse_join_game.cpp"
#include "apocalypse_deck_selector.cpp"
#include "apocalypse_render_group.cpp"
#include "apocalypse_bitmap.cpp"
#include "apocalypse_wav.cpp"
#include "apocalypse_assets.cpp"
#include "apocalypse_audio.cpp"
#include "apocalypse_particles.cpp"
#include "apocalypse_card_game.cpp"
#include "apocalypse_main_menu.cpp"
#include "apocalypse_button.cpp"
#include "apocalypse_text_input.cpp"
#include "apocalypse_deck_editor.cpp"
#include "apocalypse_deck_storage.cpp"
#include "apocalypse_card_definitions.cpp"
#include "apocalypse_info_card.cpp"
#include "apocalypse_alert.cpp"
#include "apocalypse_scroll.cpp"
#include "apocalypse_sequence_alignment.cpp"

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

void GameInitMemory(
	game_memory* Memory,
	platform_job_queue* JobQueue,
	uint32_t WindowWidth,
	uint32_t WindowHeight
)
{
	ASSERT(sizeof(game_state) <= Memory->PermanentStorageSize);
	if(!Memory->IsInitialized)
	{
		game_state* GameState = (game_state*) Memory->PermanentStorage;
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
		size_t TransientStorageDivision = Memory->TransientStorageSize / 7;
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
			&GameState->NetworkArena,
			TransientStorageDivision,
			GetEndOfArena(&GameState->FrameArena)
		);
		InitMemArena(
			&GameState->SceneArgsArena,
			TransientStorageDivision,
			GetEndOfArena(&GameState->NetworkArena)
		);
		// TODO: Asset arena probably needs to be bigger than most arenas
		memory_arena AssetArena = {};
		InitMemArena(
			&AssetArena,
			TransientStorageDivision,
			GetEndOfArena(&GameState->SceneArgsArena)
		);
		memory_arena JobArena = {};
		InitMemArena(
			&JobArena,
			TransientStorageDivision,
			GetEndOfArena(&AssetArena)
		);

		GameState->WorldToCamera = MakeBasis(
			Vector2(WindowWidth / 2.0f, WindowHeight / 2.0f),
			Vector2(1.0f, 0.0f),
			Vector2(0.0f, 1.0f)
		);
		GameState->CameraToScreen = MakeBasis(
			Vector2(
				-1.0f * WindowWidth / 2.0f, -1.0f * WindowHeight / 2.0f
			),
			Vector2(1, 0),
			Vector2(0, 1)
		);

		render_group* RenderGroup = &GameState->RenderGroup;
		*RenderGroup = {};
		RenderGroup->Arena = &GameState->RenderArena;

		RenderGroup->WorldToCamera = &GameState->WorldToCamera;
		RenderGroup->CameraToScreen = &GameState->CameraToScreen;
		AddClipRect(
			RenderGroup,
			MakeRectangle(
				Vector2(0, 0),
				Vector2(WindowWidth, WindowHeight)
			)
		);
		GameState->Time = 0;
		GameState->JobQueue = JobQueue;

		assets* Assets = &GameState->Assets; 
		*Assets = {}; 
		Assets->Arena = AssetArena;
		Assets->JobArena = JobArena;
		Assets->JobQueue = GameState->JobQueue;
		Assets->ArenaLock = PushStruct(
			&GameState->Arena, platform_mutex_handle
		);
		PlatformCreateMutex(Assets->ArenaLock);
		Assets->AvailableListLock = PushStruct(
			&GameState->Arena, platform_mutex_handle
		);
		PlatformCreateMutex(Assets->AvailableListLock);

		Memory->IsInitialized = true;

		StartMainMenu(GameState, WindowWidth, WindowHeight);
		GameState->Scene = SceneType_MainMenu;
		GameState->LastFrameScene = GameState->Scene;

		GameState->FrameCount = 0;

		PlatformMakeDirectory(DECKS_PATH);
		PlatformMakeDirectory(LOGS_PATH);
	}
}

void GameUpdateAndRender(
	game_memory* Memory,
	uint32_t WindowWidth,
	uint32_t WindowHeight,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
)
{
	TIMED_BLOCK();

	ASSERT(sizeof(game_state) <= Memory->PermanentStorageSize);
	game_state* GameState = (game_state*) Memory->PermanentStorage;

	GameState->Time += DtForFrame;

	if(GameState->LastFrameScene != GameState->Scene)
	{
		GameState->FrameCount = 0;
		switch(GameState->Scene)
		{
			case(SceneType_CardGame):
			{
				StartCardGame(GameState, WindowWidth, WindowHeight);
				break;
			}
			case(SceneType_MainMenu):
			{
				StartMainMenu(GameState, WindowWidth, WindowHeight);
				break;
			}
			case(SceneType_DeckEditor):
			{
				StartDeckEditor(GameState, WindowWidth, WindowHeight);
				break;
			}
			case(SceneType_DeckSelector):
			{
				StartDeckSelector(GameState, WindowWidth, WindowHeight);
				break;
			}
			case(SceneType_HostGame):
			{
				StartHostGame(GameState, WindowWidth, WindowHeight);
				break;
			}
			case(SceneType_JoinGame):
			{
				StartJoinGame(GameState, WindowWidth, WindowHeight);
				break;
			}
			default:
			{
				ASSERT(false);
			}
		}
		ResetMemArena(&GameState->SceneArgsArena);	
	}

	switch(GameState->Scene)
	{
		case(SceneType_CardGame):
		{
			UpdateAndRenderCardGame(
				GameState,
				(card_game_state*) GameState->SceneState,
				WindowWidth,
				WindowHeight,
				MouseEvents,
				KeyboardEvents,
				DtForFrame
			);
			GameState->LastFrameScene = SceneType_CardGame;
			break;
		}
		case(SceneType_MainMenu):
		{
			UpdateAndRenderMainMenu(
				GameState,
				(main_menu_state*) GameState->SceneState,
				WindowWidth,
				WindowHeight,
				MouseEvents,
				KeyboardEvents,
				DtForFrame
			);
			GameState->LastFrameScene = SceneType_MainMenu;
			break;
		}
		case(SceneType_DeckEditor):
		{
			UpdateAndRenderDeckEditor(
				GameState,
				(deck_editor_state*) GameState->SceneState,
				WindowWidth,
				WindowHeight,
				MouseEvents,
				KeyboardEvents,
				DtForFrame
			);
			GameState->LastFrameScene = SceneType_DeckEditor;
			break;
		}
		case(SceneType_DeckSelector):
		{
			UpdateAndRenderDeckSelector(
				GameState,
				(deck_selector_state*) GameState->SceneState,
				WindowWidth,
				WindowHeight,
				MouseEvents,
				KeyboardEvents,
				DtForFrame
			);
			GameState->LastFrameScene = SceneType_DeckSelector;
			break;
		}
		case(SceneType_HostGame):
		{
			UpdateAndRenderHostGame(
				GameState,
				(host_game_state*) GameState->SceneState,
				WindowWidth,
				WindowHeight,
				MouseEvents,
				KeyboardEvents,
				DtForFrame
			);
			GameState->LastFrameScene = SceneType_HostGame;
			break;
		}
		case(SceneType_JoinGame):
		{
			UpdateAndRenderJoinGame(
				GameState,
				(join_game_state*) GameState->SceneState,
				WindowWidth,
				WindowHeight,
				MouseEvents,
				KeyboardEvents,
				DtForFrame
			);
			GameState->LastFrameScene = SceneType_JoinGame;
			break;
		}
		default:
		{
			ASSERT(false);
		}
	}
	
	RenderGroupToOutput(&GameState->RenderGroup, WindowWidth, WindowHeight);

	ResetMemArena(&GameState->FrameArena);

	GameState->FrameCount++;
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

void HandleGameDebug(
	game_memory* Memory, uint32_t WindowWidth, uint32_t WindowHeight
)
{
	ASSERT(sizeof(game_state) <= Memory->PermanentStorageSize);
	game_state* GameState = (game_state*) Memory->PermanentStorage;
	if(GameState->OverlayDebugInfo)
	{
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
				WindowHeight - 30.0f
			),
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			&GameState->FrameArena
		);

		RenderGroupToOutput(
			&GameState->RenderGroup, WindowWidth, WindowHeight
		);
		ResetMemArena(&GameState->FrameArena);
	}
}