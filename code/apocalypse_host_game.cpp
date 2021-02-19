#include "apocalypse_host_game.h"
#include "apocalypse.h"
#include "apocalypse_assets.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_platform.h"
#include "apocalypse_render_group.h"
#include "apocalypse_socket.h"
#include "apocalypse_card_game.h"
#include "apocalypse_deck_selector.h"

void StartHostGamePrep(game_state* GameState)
{
	GameState->SceneArgs = NULL;
	GameState->Scene = SceneType_HostGame; 
}

void StartHostGame(
	game_state* GameState, uint32_t WindowWidth, uint32_t WindowHeight
)
{
	GameState->SceneState = PushStruct(
		&GameState->TransientArena, host_game_state
	);

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

			join_game_type_packet* Packet = PushStruct(
				FrameArena, join_game_type_packet
			);
			join_game_type_payload* Payload = &Packet->Payload;
			Payload->Type = JoinGameType_NewGame;

			packet_header* Header = &Packet->Header; 
			Header->DataSize = sizeof(join_game_type_packet);
			InitPacketHeader(
				GameState, Header, Packet_JoinGameType, (uint8_t*) Payload
			);
			SocketSendErrorCheck(
				GameState,
				SceneState->ClientSocket,
				SceneState->ListenSocket,
				Header
			);
		}
	}
	else
	{
		StartDeckSelectorPrep(
			GameState,
			SceneType_CardGame,
			true,
			true,
			SceneState->ListenSocket,
			SceneState->ClientSocket
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
}