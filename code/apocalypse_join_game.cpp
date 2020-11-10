#include "apocalypse_join_game.h"
#include "apocalypse.h"
#include "apocalypse_assets.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_platform.h"
#include "apocalypse_render_group.h"

void StartJoinGamePrep(game_state* GameState)
{
	GameState->SceneArgs = NULL;
	GameState->Scene = SceneType_JoinGame; 
}

void StartJoinGame(
	game_state* GameState, uint32_t WindowWidth, uint32_t WindowHeight
)
{
	ResetMemArena(&GameState->TransientArena);
	GameState->SceneState = PushStruct(
		&GameState->TransientArena, join_game_state
	);
	ResetAssets(&GameState->Assets);

	join_game_state* SceneState = (join_game_state*) GameState->SceneState;
	*SceneState = {};

	SceneState->ScreenDimInWorld = TransformVectorToBasis(
		&GameState->WorldToCamera,
		Vector2(WindowWidth, WindowHeight)
	);

	// TODO: move this out to a thread
	platform_socket* ConnectSocket = PushStruct(
		&GameState->TransientArena, platform_socket
	);

	platform_socket_result ServerResult = PlatformCreateClient(
		"127.0.0.1", ConnectSocket
	);
	if(ServerResult != PlatformSocketResult_Success)
	{
		// TODO: logging
		ASSERT(false);
	}
}

void UpdateAndRenderJoinGame(
	game_state* GameState,
	join_game_state* SceneState,
	uint32_t WindowWidth,
	uint32_t WindowHeight,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
)
{
	memory_arena* FrameArena = &GameState->FrameArena;
	render_group* RenderGroup = &GameState->RenderGroup;
	assets* Assets = &GameState->Assets;

	PushText(
		RenderGroup,
		Assets,
		FontHandle_TestFont,
		"Connected to server",
		64,
		50.0f,
		SceneState->ScreenDimInWorld / 2.0f,
		Vector4(1.0f, 1.0f, 1.0f, 1.0f),
		FrameArena
	);
}