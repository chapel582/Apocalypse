#include "apocalypse_host_game.h"
#include "apocalypse.h"
#include "apocalypse_assets.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_platform.h"
#include "apocalypse_render_group.h"

void StartHostGamePrep(game_state* GameState)
{
	GameState->SceneArgs = NULL;
	GameState->Scene = SceneType_HostGame; 
}

void StartHostGame(
	game_state* GameState, uint32_t WindowWidth, uint32_t WindowHeight
)
{
	ResetMemArena(&GameState->TransientArena);
	GameState->SceneState = PushStruct(
		&GameState->TransientArena, host_game_state
	);
	ResetAssets(&GameState->Assets);

	host_game_state* SceneState = (host_game_state*) GameState->SceneState;
	*SceneState = {};

	SceneState->ScreenDimInWorld = TransformVectorToBasis(
		&GameState->WorldToCamera,
		Vector2(WindowWidth, WindowHeight)
	);

	// TODO: spawn a thread that does this
	platform_socket* ListenSocket = PushStruct(
		&GameState->TransientArena, platform_socket
	);
	platform_socket* ClientSocket = PushStruct(
		&GameState->TransientArena, platform_socket
	);

	platform_socket_result ServerResult = PlatformCreateServer(
		ListenSocket, ClientSocket
	);
	if(ServerResult != PlatformSocketResult_Success)
	{
		// TODO: logging
		ASSERT(false);
	}
}

void UpdateAndRenderHostGame(
	game_state* GameState,
	host_game_state* SceneState,
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
		"Connected to client",
		64,
		50.0f,
		SceneState->ScreenDimInWorld / 2.0f,
		Vector4(1.0f, 1.0f, 1.0f, 1.0f),
		FrameArena
	);
}