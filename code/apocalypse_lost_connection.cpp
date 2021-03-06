#include "apocalypse.h"
#include "apocalypse_assets.h"
#include "apocalypse_render_group.h"
#include "apocalypse_lost_connection.h"
#include "apocalypse_card_game.h"
#include "apocalypse_join_game.h"

void StartLostConnectionPrep(
	game_state* GameState,
	platform_socket* ConnectSocket,
	platform_socket* ListenSocket
)
{
	// NOTE: right now, this assumes you want this scene stacked atop the old 
	// CONT: scene
	// NOTE: lost connection scenes cannot be stacked atop themselves 
	if(GameState->Scene != SceneType_LostConnection)
	{
		AddSceneStackEntry(GameState);
		GameState->Scene = SceneType_LostConnection;
	}

	lost_connection_args* SceneArgs = PushStruct(
		&GameState->SceneArgsArena, lost_connection_args
	);
	*SceneArgs = {};

	SceneArgs->ConnectSocket = *ConnectSocket;
	if(ListenSocket)
	{
		SceneArgs->ListenSocket = *ListenSocket;
	}
	else
	{
		SceneArgs->ListenSocket = {};
	}
	GameState->SceneArgs = SceneArgs;
}

void StartLostConnection(
	game_state* GameState, uint32_t WindowWidth, uint32_t WindowHeight
)
{
	lost_connection_args* SceneArgs = (lost_connection_args*) (
		GameState->SceneArgs
	);

	GameState->SceneState = PushStruct(
		&GameState->TransientArena, lost_connection_state
	);
	lost_connection_state* SceneState = (lost_connection_state*)(
		GameState->SceneState
	);
	SceneState->ListenSocket = SceneArgs->ListenSocket;
	SceneState->ConnectSocket = SceneArgs->ConnectSocket;
	PlatformCloseSocket(&SceneState->ConnectSocket);
	
	// NOTE: if you weren't the server, YOU ARE NOW
	// CONT: time to step up, little client
	if(!SceneState->ListenSocket.IsValid)
	{
		platform_socket_result ServerResult = PlatformCreateListen(
			&SceneState->ListenSocket
		);
		if(ServerResult != PlatformSocketResult_Success)
		{
			// TODO: logging
			ASSERT(false);
		}
	}

	ui_context* UiContext = &SceneState->UiContext;
	InitUiContext(UiContext);
}

void UpdateAndRenderLostConnection(
	game_state* GameState,
	lost_connection_state* SceneState,
	uint32_t WindowWidth,
	uint32_t WindowHeight,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
)
{

	memory_arena* FrameArena = &GameState->FrameArena;

	platform_socket_result SocketResult = PlatformAcceptConnection(
		&SceneState->ListenSocket, &SceneState->ConnectSocket
	);
	if(SocketResult == PlatformSocketResult_Success)
	{
		scene_stack_entry* StackEntry = PeekSceneStack(GameState);
		switch(StackEntry->Scene)
		{
			case(SceneType_CardGame):
			{
				card_game_state* CardGameState = (card_game_state*)(
					StackEntry->SceneState
				);
				// NOTE: assigning the listen socket is necessary for former 
				// CONT: clients that have become servers
				CardGameState->ListenSocket = SceneState->ListenSocket;
				CardGameState->ConnectSocket = SceneState->ConnectSocket;
				CardGameState->SyncState = SyncState_Send;

				{
					join_game_type_packet* Packet = PushStruct(
						FrameArena, join_game_type_packet
					);
					join_game_type_payload* Payload = &Packet->Payload;
					Payload->Type = JoinGameType_ResumeGame;

					packet_header* Header = &Packet->Header; 
					Header->DataSize = sizeof(join_game_type_packet);
					InitPacketHeader(
						GameState,
						Header,
						Packet_JoinGameType,
						(uint8_t*) Payload
					);
					SocketSendErrorCheck(
						GameState,
						&SceneState->ConnectSocket,
						&SceneState->ListenSocket,
						Header
					);
				}

				// NOTE: next share the seed with the rejoining game
				{
					rand_seed_packet* RandSeedPacket = PushStruct(
						FrameArena, rand_seed_packet
					);
					rand_seed_payload* Payload = &RandSeedPacket->Payload;
					Payload->Seed = CardGameState->Seed;

					packet_header* Header = &RandSeedPacket->Header; 
					Header->DataSize = sizeof(rand_seed_packet);
					InitPacketHeader(
						GameState,
						Header,
						Packet_RandSeed,
						(uint8_t*) Payload
					);
					SocketSendErrorCheck(
						GameState,
						&SceneState->ConnectSocket,
						&SceneState->ListenSocket,
						Header
					);
				}
				break;
			}
			default:
			{
				ASSERT(false);
			}
		}
		GameState->PopScene = true;
	}
	
	user_event_index UserEventIndex = 0;
	int MouseEventIndex = 0;
	int KeyboardEventIndex = 0;
	while(
		(MouseEventIndex < MouseEvents->Length) ||
		(KeyboardEventIndex < KeyboardEvents->Length)
	)
	{
		for(; MouseEventIndex < MouseEvents->Length; MouseEventIndex++)
		{
			game_mouse_event* MouseEvent = (
				&MouseEvents->Events[MouseEventIndex]
			);

			if(MouseEvent->UserEventIndex != UserEventIndex)
			{
				break;
			}

			vector2 MouseEventWorldPos = TransformPosFromBasis(
				&GameState->WorldToCamera,
				TransformPosFromBasis(
					&GameState->CameraToScreen, 
					Vector2(MouseEvent->XPos, MouseEvent->YPos)
				)
			);
			
			UserEventIndex++;
		}

		for(; KeyboardEventIndex < KeyboardEvents->Length; KeyboardEventIndex++)
		{
			game_keyboard_event* KeyboardEvent = (
				&KeyboardEvents->Events[KeyboardEventIndex]
			);
			if(KeyboardEvent->UserEventIndex != UserEventIndex)
			{
				break;
			}

			if(KeyboardEvent->IsDown != KeyboardEvent->WasDown)
			{
				switch(KeyboardEvent->Code)
				{
					case(0x1B): // NOTE: Escape V-code
					{
						GameState->Scene = SceneType_MainMenu; 

						// NOTE: clean up the sockets now that we're going back
						// CONT: to main menu 
						platform_socket* ListenSocket = (
							&SceneState->ListenSocket
						);
						platform_socket* ConnectSocket = (
							&SceneState->ConnectSocket
						);
						if(
							ListenSocket->IsValid
						)
						{
							PlatformServerDisconnect(
								ListenSocket, ConnectSocket
							);
						}
						else
						{
							PlatformClientDisconnect(ConnectSocket);
						}
						break;
					}
				}
			}

			UserEventIndex++;
		}
	}

	render_group* RenderGroup = &GameState->RenderGroup;
	PushClear(RenderGroup, Vector4(0.25f, 0.25f, 0.25f, 1.0f));

	vector4 Black = Vector4(0.0f, 0.0f, 0.0f, 1.0f);

	assets* Assets = &GameState->Assets;
	PushText(
		RenderGroup,
		Assets,
		FontHandle_TestFont,
		"Peer reset connection unexpectedly.\n\"ESC\" to return to main menu",
		sizeof("Peer reset connection unexpectedly.\n\"ESC\" to return to main menu"),
		50.0f,
		Vector2(WindowWidth / 2.0f, WindowHeight / 2.0f),
		Vector4(1.0f, 1.0f, 1.0f, 1.0f),
		FrameArena
	);	
}