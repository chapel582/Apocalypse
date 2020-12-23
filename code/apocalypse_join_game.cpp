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
	connection_args* Args = (connection_args*) Data;
	platform_socket* ConnectSocket = Args->ConnectSocket;
	
	platform_socket_result ConnectResult = PlatformCreateClient(
		Args->IpAddress, ConnectSocket
	);
	if(ConnectResult != PlatformSocketResult_Success)
	{
		// TODO: logging
		Args->ConnectionState = ConnectionState_Error;
	}
	else
	{
		Args->ConnectionState = ConnectionState_Complete;
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

	memory_arena* TransientArena = &GameState->TransientArena;
	SceneState->ConnectSocket = PushStruct(TransientArena, platform_socket);

	SceneState->ConnectionArgs = {};
	SceneState->ConnectionArgs.ConnectSocket = SceneState->ConnectSocket;
	SceneState->ConnectionArgs.ConnectionState = ConnectionState_IpEntry;

	SceneState->PacketBufferSize = 256;
	SceneState->PacketBuffer = PushSize(
		TransientArena, SceneState->PacketBufferSize
	);

	InitUiContext(&SceneState->UiContext);
	ui_context* UiContext = &SceneState->UiContext;
	InitTextInput(
		UiContext,
		&SceneState->IpInput,
		20.0f,
		SceneState->ConnectionArgs.IpAddress,
		sizeof(SceneState->ConnectionArgs.IpAddress)
	);
	SceneState->IpInputRectangle = MakeRectangleCentered(
		Vector2(WindowWidth / 2.0f, WindowHeight / 2.0f),
		Vector2(2.0f * WindowWidth / 5.0f, SceneState->IpInput.FontHeight)
	);
	SetActive(UiContext, SceneState->IpInput.UiId);
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
	ui_context* UiContext = &SceneState->UiContext;

	uint32_t BytesRead = 0;
	if(
		SceneState->ConnectionArgs.ConnectionState == ConnectionState_IpEntry
	)
	{
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
				
				TextInputHandleMouse(
					UiContext,
					&SceneState->IpInput,
					SceneState->IpInputRectangle,
					MouseEvent,
					MouseEventWorldPos
				);				

				UserEventIndex++;
			}

			for(
				;
				KeyboardEventIndex < KeyboardEvents->Length;
				KeyboardEventIndex++
			)
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
					// NOTE: :common keyboard actions across all states
					switch(KeyboardEvent->Code)
					{
						case(0x1B): // NOTE: Escape V-code
						{
							GameState->Scene = SceneType_MainMenu; 
							break;
						}
					}

					text_input_kb_result KeyboardResult = (TextInputHandleKeyboard(
							UiContext,
							&SceneState->IpInput,
							KeyboardEvent
						)
					);
					if(KeyboardResult == TextInputKbResult_Submit)
					{
						SceneState->ConnectionArgs.ConnectionState = (
							ConnectionState_InProgress
						);
						PlatformAddJob(
							GameState->JobQueue,
							ConnectJob,
							&SceneState->ConnectionArgs,
							JobPriority_Other
						);
					}
				}

				UserEventIndex++;
			}
		}
	}
	else if(
		SceneState->ConnectionArgs.ConnectionState == ConnectionState_Complete
	)
	{
		// TODO: probably want to just join a thread/check for a particular job
		// CONT: finishing, instead of all jobs
		PlatformCompleteAllJobs(GameState->JobQueue);
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

	if(
		SceneState->ConnectionArgs.ConnectionState == ConnectionState_IpEntry
	)
	{
		vector4 FontColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		vector4 BackgroundColor = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
		bitmap_handle Background = BitmapHandle_TestCard2;
		PushTextInput(
			UiContext,
			&SceneState->IpInput,
			FontColor,
			SceneState->IpInputRectangle,
			Background,
			BackgroundColor,
			Assets,
			RenderGroup,
			FrameArena
		);
	}
	else if(
		SceneState->ConnectionArgs.ConnectionState == ConnectionState_InProgress
	)
	{
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
	else if(SceneState->ConnectionArgs.ConnectionState == ConnectionState_Error)
	{
		// TODO: recovery and logging
		ASSERT(false);
	}
}