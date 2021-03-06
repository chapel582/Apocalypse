#include "apocalypse.h"
#include "apocalypse_assets.h"
#include "apocalypse_render_group.h"
#include "apocalypse_card_game.h"
#include "apocalypse_deck_editor.h"
#include "apocalypse_button.h"
#include "apocalypse_deck_selector.h"
#include "apocalypse_host_game.h"
#include "apocalypse_join_game.h"
#include "apocalypse_options_menu.h"

void StartMainMenu(
	game_state* GameState, uint32_t WindowWidth, uint32_t WindowHeight
)
{
	GameState->SceneState = PushStruct(
		&GameState->TransientArena, main_menu_state
	);
	main_menu_state* SceneState = (main_menu_state*) GameState->SceneState;
	
	ui_context* UiContext = &SceneState->UiContext;
	InitUiContext(UiContext);
	vector2 Center = Vector2(WindowWidth / 2.0f, WindowHeight / 2.0f);
	vector2 Dim = Vector2(WindowWidth / 5.5f, WindowHeight / 20.0f); 
	vector2 ButtonMin = Center - 0.5f * Dim;
	InitButton(
		UiContext,
		&SceneState->CardGameButton,
		MakeTrackedRectangle(GameState, ButtonMin, Dim)
	);

	ButtonMin.Y -= 1.5f * Dim.Y;
	InitButton(
		UiContext,
		&SceneState->HostGameButton,
		MakeTrackedRectangle(GameState, ButtonMin, Dim)
	);
	
	ButtonMin.Y -= 1.5f * Dim.Y;
	InitButton(
		UiContext,
		&SceneState->JoinGameButton,
		MakeTrackedRectangle(GameState, ButtonMin, Dim)
	);
	
	ButtonMin.Y -= 1.5f * Dim.Y;
	InitButton(
		UiContext,
		&SceneState->DeckEditorButton,
		MakeTrackedRectangle(GameState, ButtonMin, Dim)
	);

	ButtonMin.Y -= 1.5f * Dim.Y;
	InitButton(
		UiContext,
		&SceneState->OptionsButton,
		MakeTrackedRectangle(GameState, ButtonMin, Dim)
	);
}

void UpdateAndRenderMainMenu(
	game_state* GameState,
	main_menu_state* SceneState,
	uint32_t WindowWidth,
	uint32_t WindowHeight,
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

			button_handle_event_result Result = ButtonHandleEvent(
				&SceneState->UiContext,
				&SceneState->CardGameButton,
				MouseEvent->Type,
				MouseEventWorldPos
			);
			if(Result == ButtonHandleEvent_TakeAction)
			{
				StartDeckSelectorPrep(
					GameState,
					SceneType_CardGame,
					false,
					false,
					NULL,
					NULL
				);
			}
			Result = ButtonHandleEvent(
				&SceneState->UiContext,
				&SceneState->HostGameButton,
				MouseEvent->Type,
				MouseEventWorldPos
			);
			if(Result == ButtonHandleEvent_TakeAction)
			{
				StartHostGamePrep(GameState);
			}
			Result = ButtonHandleEvent(
				&SceneState->UiContext,
				&SceneState->JoinGameButton,
				MouseEvent->Type,
				MouseEventWorldPos
			);
			if(Result == ButtonHandleEvent_TakeAction)
			{
				StartJoinGamePrep(GameState);
			}
			Result = ButtonHandleEvent(
				&SceneState->UiContext,
				&SceneState->DeckEditorButton,
				MouseEvent->Type,
				MouseEventWorldPos
			);
			if(Result == ButtonHandleEvent_TakeAction)
			{
				StartDeckSelectorPrep(
					GameState,
					SceneType_DeckEditor,
					false,
					false,
					NULL,
					NULL
				);
			}
			Result = ButtonHandleEvent(
				&SceneState->UiContext,
				&SceneState->OptionsButton,
				MouseEvent->Type,
				MouseEventWorldPos
			);
			if(Result == ButtonHandleEvent_TakeAction)
			{
				StartOptionsMenuPrep(GameState);	
			}
			
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

			UserEventIndex++;
		}
	}

	PushClear(&GameState->RenderGroup, Vector4(0.25f, 0.25f, 0.25f, 1.0f));

	vector4 Black = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	PushButtonToRenderGroup(
		*SceneState->CardGameButton.Rectangle,
		BitmapHandle_TestCard2,
		&GameState->RenderGroup,
		&GameState->Assets, 
		"Start Card Game",
		sizeof("Start Card Game"),
		FontHandle_TestFont,
		Black, 
		&GameState->FrameArena
	);
	PushButtonToRenderGroup(
		*SceneState->HostGameButton.Rectangle,
		BitmapHandle_TestCard2,
		&GameState->RenderGroup,
		&GameState->Assets, 
		"Host Card Game",
		sizeof("Host Card Game"),
		FontHandle_TestFont,
		Black, 
		&GameState->FrameArena
	);
	PushButtonToRenderGroup(
		*SceneState->JoinGameButton.Rectangle,
		BitmapHandle_TestCard2,
		&GameState->RenderGroup,
		&GameState->Assets, 
		"Join Card Game",
		sizeof("Join Card Game"),
		FontHandle_TestFont,
		Black, 
		&GameState->FrameArena
	);
	PushButtonToRenderGroup(
		*SceneState->DeckEditorButton.Rectangle,
		BitmapHandle_TestCard2,
		&GameState->RenderGroup,
		&GameState->Assets, 
		"Deck Editor",
		sizeof("Deck Editor"),
		FontHandle_TestFont,
		Black,
		&GameState->FrameArena
	);
	PushButtonToRenderGroup(
		*SceneState->OptionsButton.Rectangle,
		BitmapHandle_TestCard2,
		&GameState->RenderGroup,
		&GameState->Assets, 
		"Options",
		sizeof("Options"),
		FontHandle_TestFont,
		Black,
		&GameState->FrameArena
	);
}