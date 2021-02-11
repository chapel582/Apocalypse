#include "apocalypse.h"
#include "apocalypse_assets.h"
#include "apocalypse_render_group.h"
#include "apocalypse_lost_connection.h"

void StartLostConnectionPrep(
	game_state* GameState,
	platform_socket* ConnectSocket,
	platform_socket* ListenSocket
)
{
	GameState->Scene = SceneType_LostConnection;
	if(ListenSocket->IsValid && ConnectSocket->IsValid)
	{
		PlatformServerDisconnect(ListenSocket, ConnectSocket);
	}
	else if(ConnectSocket->IsValid)
	{
		PlatformClientDisconnect(ConnectSocket);
	}
	else
	{
		ASSERT(false);
	}
}

void StartLostConnection(
	game_state* GameState, uint32_t WindowWidth, uint32_t WindowHeight
)
{
	ResetMemArena(&GameState->TransientArena);
	GameState->SceneState = PushStruct(
		&GameState->TransientArena, lost_connection_state
	);
	ResetAssets(&GameState->Assets);
	lost_connection_state* SceneState = (lost_connection_state*)(
		GameState->SceneState
	);

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