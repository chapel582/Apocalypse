#include "apocalypse.h"
#include "apocalypse_assets.h"
#include "apocalypse_render_group.h"
#include "apocalypse_card_game.h"
#include "apocalypse_button.h"

void StartMainMenu(game_state* GameState, game_offscreen_buffer* BackBuffer)
{
	ResetMemArena(&GameState->TransientArena);
	GameState->SceneState = PushStruct(
		&GameState->TransientArena, main_menu_state
	);
	ResetAssets(&GameState->Assets);
	main_menu_state* SceneState = (main_menu_state*) GameState->SceneState;
	
	InitButtons(SceneState->Buttons, ARRAY_COUNT(SceneState->Buttons));

	start_game_args* StartGameArgs = PushStruct(
		&GameState->TransientArena, start_game_args
	);
	vector2 Center = Vector2(
		BackBuffer->Width / 2.0f, BackBuffer->Height / 2.0f
	);
	vector2 Dim = Vector2(BackBuffer->Width / 4.0f, BackBuffer->Height / 10.0f); 
	AddButton(
		SceneState->Buttons,
		ARRAY_COUNT(SceneState->Buttons),
		MakeRectangle(Center - 0.5f * Dim, Dim),
		BitmapHandle_TestCard,
		FontHandle_TestFont,
		"Start Card Game",
		StartCardGameCallback,
		StartGameArgs
	);
	
	GameState->Scene = SceneType_MainMenu;
}

void UpdateAndRenderMainMenu(
	game_state* GameState,
	main_menu_state* SceneState,
	game_offscreen_buffer* BackBuffer,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
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

			ButtonsHandleMouseEvent(
				SceneState->Buttons,
				ARRAY_COUNT(SceneState->Buttons),
				MouseEvent,
				MouseEventWorldPos
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
				
			}

			UserEventIndex++;
		}
	}

	PushClear(&GameState->RenderGroup, Vector4(0.25f, 0.25f, 0.25f, 1.0f));
	PushTextCentered(
		&GameState->RenderGroup,
		&GameState->Assets,
		FontHandle_TestFont,
		"You have switched scenes!",
		50,
		50.0f,
		Vector2(
			BackBuffer->Width / 2.0f, 
			(BackBuffer->Height / 2.0f)
		),
		Vector4(1.0f, 1.0f, 1.0f, 1.0f),
		&GameState->FrameArena
	);

	PushButtonsToRenderGroup(
		SceneState->Buttons,
		ARRAY_COUNT(SceneState->Buttons),
		&GameState->RenderGroup,
		&GameState->Assets, 
		&GameState->FrameArena
	);
}