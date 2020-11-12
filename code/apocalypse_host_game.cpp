#include "apocalypse_host_game.h"
#include "apocalypse.h"
#include "apocalypse_assets.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_platform.h"
#include "apocalypse_render_group.h"
#include "apocalypse_socket.h"
#include "apocalypse_card_game.h"

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

	SceneState->ListenSocket = PushStruct(
		&GameState->TransientArena, platform_socket
	);
	*SceneState->ListenSocket = {};
	SceneState->ClientSocket = PushStruct(
		&GameState->TransientArena, platform_socket
	);
	*SceneState->ClientSocket = {};

	// TODO: spawn a thread that waits for the connection
	platform_socket_result ServerResult = PlatformCreateListen(
		SceneState->ListenSocket
	);
	if(ServerResult != PlatformSocketResult_Success)
	{
		// TODO: logging
		ASSERT(false);
	}
	SceneState->Listening = true;

	SceneState->SendDataArgs = PushStruct(
		&GameState->TransientArena, socket_send_data_args
	);
	SceneState->SendDataArgs->Socket = SceneState->ClientSocket;
	SceneState->SendDataArgs->BufferSize = 256;
	SceneState->SendDataArgs->Buffer = PushSize(
		&GameState->TransientArena,
		SceneState->SendDataArgs->BufferSize
	);
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

	if(SceneState->Listening)
	{
		platform_socket_result SocketResult = PlatformAcceptConnection(
			SceneState->ListenSocket, SceneState->ClientSocket
		);
		if(SocketResult == PlatformSocketResult_Success)
		{
			SceneState->Listening = false;
		}
	}
	else
	{
		char* CharBuffer = (char*) SceneState->SendDataArgs->Buffer;
		*CharBuffer++ = 'a';
		*CharBuffer++ = 'b';
		*CharBuffer++ = 'c';
		*CharBuffer++ = '1';
		*CharBuffer++ = '2';
		*CharBuffer++ = '3';
		*CharBuffer++ = 0;

		SceneState->SendDataArgs->DataSize = 7;
		PlatformAddJob(
			GameState->JobQueue,
			SocketSendDataJob,
			SceneState->SendDataArgs,
			JobPriority_SendPacket
		);
	}

	PushClear(RenderGroup, Vector4(0.25f, 0.25f, 0.25f, 1.0f));
	PushText(
		RenderGroup,
		Assets,
		FontHandle_TestFont,
		"Waiting for client",
		64,
		50.0f,
		SceneState->ScreenDimInWorld / 2.0f,
		Vector4(1.0f, 1.0f, 1.0f, 1.0f),
		FrameArena
	);

	PlatformCompleteAllJobsAtPriority(
		GameState->JobQueue, JobPriority_SendPacket
	);
}