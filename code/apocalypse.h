#ifndef APOCALYPSE_H

#include "apocalypse_platform.h"
#include "apocalypse_vector.h"
#include "apocalypse_rectangle.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_bitmap.h"
#include "apocalypse_render_group.h"
#include "apocalypse_assets.h"
#include "apocalypse_audio.h"
#include "apocalypse_particles.h"

typedef enum
{
	JobPriority_SendPacket,
	JobPriority_Asset,
	JobPriority_Other
} default_job_priorities;

typedef enum
{
	SceneType_CardGame,
	SceneType_MainMenu,
	SceneType_DeckEditor,
	SceneType_DeckSelector,
	SceneType_HostGame,
	SceneType_JoinGame,
	SceneType_OptionsMenu,
	SceneType_LostConnection
} scene_type;

struct scene_stack_entry
{
	scene_type Scene;
	size_t ResetTransientTo;
	size_t ResetRectanglesTo;
};

struct game_state
{
	// NOTE: Arena is just for permanent things that can/should be determined
	// CONT: at run time
	memory_arena Arena;
	// NOTE: TransientArena is for memory that last longer than a frame but 
	// CONT: can be cleaned up in a big batch, usually when switching contexts 
	// CONT: in the game, e.g. from the menu to starting the game 
	memory_arena TransientArena;
	// NOTE: FrameArena is just for memory that will not be needed after EOF
	memory_arena FrameArena;
	// NOTE: NetworkArena is an arena for network packets after they arrive 
	memory_arena NetworkArena;
	// NOTE: render arena is just for the renderer
	memory_arena RenderArena;
	// NOTE: an arena for scene args
	memory_arena SceneArgsArena;
	// NOTE: an arena for tracked rectangles. cleared with new context
	memory_arena TrackedRectanglesArena;

	// NOTE: number of frames that have passed in this scene
	uint64_t FrameCount;

	// NOTE: the state data needed in order to get the next packet id in 
	// CONT: networked environments
	uint64_t PacketIdTracker;

	platform_job_queue* JobQueue;

	assets Assets;

	render_group RenderGroup;
	basis WorldToCamera;
	basis CameraToScreen;

	playing_sound_list PlayingSoundList;

	float Time;

	scene_type Scene;
	scene_type LastFrameScene;
	// TODO: consider whether there's a way for users to go arbitrarily deep on
	// CONT: the stack, and how to handle that
	scene_stack_entry SceneStack[10];
	uint32_t SceneStackLen;
	uint32_t SceneStackMaxLen;
	bool EmptyStack; // NOTE: a command set by the previous frame's state
	bool PopScene;
	void* SceneState;
	void* SceneArgs;

	bool OverlayDebugInfo;

	uint64_t UpdateDelay; // NOTE: in frames 
	bool CanSendPackets;

	// NOTE: tracked rectangles are rectangles that can be used to easily update
	// CONT: their positions after a change in window size
	rectangle* TrackedRectangles;
	uint32_t TrackedRectanglesCount;
};

rectangle* MakeTrackedRectangle(game_state* GameState, vector2 Min, vector2 Dim)
{
	rectangle* Result = PushStruct(
		&GameState->TrackedRectanglesArena, rectangle
	);
	GameState->TrackedRectanglesCount++;
	*Result = MakeRectangle(Min, Dim);
	return Result;
}

void SetWindowSize(
	game_state* GameState,
	uint32_t OldWindowWidth,
	uint32_t OldWindowHeight,
	uint32_t NewWindowWidth,
	uint32_t NewWindowHeight
);

void AddSceneStackEntry(game_state* GameState);
#define APOCALYPSE_H
#endif
