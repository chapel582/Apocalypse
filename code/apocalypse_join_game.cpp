#include "apocalypse_join_game.h"
#include "apocalypse.h"
#include "apocalypse_assets.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_platform.h"
#include "apocalypse_render_group.h"
#include "apocalypse_card_game.h"
#include "apocalypse_deck_selector.h"

void ConnectJob(void* Data)
{
	connect_to_server_args* Args = (connect_to_server_args*) Data;
	platform_socket* ConnectSocket = Args->ConnectSocket;
	
	platform_socket_result ConnectResult = PlatformCreateClient(
		"127.0.0.1", ConnectSocket
	);
	if(ConnectResult != PlatformSocketResult_Success)
	{
		// TODO: logging
		Args->ConnectionResult = ConnectToServerResult_Error;
	}
	else
	{
		Args->ConnectionResult = ConnectToServerResult_Complete;
	}
}

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
	SceneState->ConnectSocket = PushStruct(
		&GameState->TransientArena, platform_socket
	);

	SceneState->ConnectToServerArgs = PushStruct(
		&GameState->TransientArena, connect_to_server_args
	);
	*SceneState->ConnectToServerArgs = {};
	SceneState->ConnectToServerArgs->ConnectSocket = SceneState->ConnectSocket;
	SceneState->ConnectToServerArgs->ConnectionResult = (
		ConnectToServerResult_InProgress
	);
	PlatformAddJob(
		GameState->JobQueue,
		ConnectJob,
		SceneState->ConnectToServerArgs,
		JobPriority_Other
	);

	SceneState->PacketBufferSize = 256;
	SceneState->PacketBuffer = PushSize(
		&GameState->TransientArena,
		SceneState->PacketBufferSize
	);
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

	uint32_t BytesRead = 0;
	if(
		SceneState->ConnectToServerArgs->ConnectionResult == 
		ConnectToServerResult_Complete
	)
	{
		StartDeckSelectorPrep(
			GameState,
			SceneType_CardGame,
			true,
			false,
			NULL,
			SceneState->ConnectSocket
		);
	}

	PushClear(RenderGroup, Vector4(0.25f, 0.25f, 0.25f, 1.0f));

	PushText(
		RenderGroup,
		Assets,
		FontHandle_TestFont,
		"Connecting to server",
		64,
		50.0f,
		SceneState->ScreenDimInWorld / 2.0f,
		Vector4(1.0f, 1.0f, 1.0f, 1.0f),
		FrameArena
	);
}