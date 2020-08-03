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
	SceneType_CardGame,
	SceneType_MainMenu,
} scene_type;

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
	// NOTE: render arena is just for the renderer
	memory_arena RenderArena;

	assets Assets;

	render_group RenderGroup;
	basis WorldToCamera;
	basis CameraToScreen;

	playing_sound_list PlayingSoundList;

	float Time;

	scene_type Scene;
	void* SceneState;

	bool OverlayDebugInfo;
};

#define APOCALYPSE_H
#endif
