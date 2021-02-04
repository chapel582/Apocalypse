#include "apocalypse.h"
#include "apocalypse_assets.h"
#include "apocalypse_render_group.h"
#include "apocalypse_options_menu.h"

void StartOptionsMenuPrep(game_state* GameState)
{
	GameState->Scene = SceneType_OptionsMenu; 
}

void StartOptionsMenu(
	game_state* GameState, uint32_t WindowWidth, uint32_t WindowHeight
)
{
	ResetMemArena(&GameState->TransientArena);
	GameState->SceneState = PushStruct(
		&GameState->TransientArena, options_menu_state
	);
	ResetAssets(&GameState->Assets);
	options_menu_state* SceneState = (options_menu_state*)(
		GameState->SceneState
	);

	uint32_t ResolutionOptionCount = 3;
	SceneState->ResolutionPairs = PushArray(
		&GameState->TransientArena, 2 * ResolutionOptionCount, uint32_t
	);
	SceneState->ResolutionPairs[0] = 800;
	SceneState->ResolutionPairs[1] = 600;
	SceneState->ResolutionPairs[2] = 1440;
	SceneState->ResolutionPairs[3] = 900;
	SceneState->ResolutionPairs[4] = 1920;
	SceneState->ResolutionPairs[5] = 1080;

	ui_context* UiContext = &SceneState->UiContext;
	InitUiContext(UiContext);
	vector2 Center = Vector2(WindowWidth / 2.0f, WindowHeight / 2.0f);
	vector2 Dim = Vector2(WindowWidth / 5.5f, WindowHeight / 20.0f); 
	vector2 DropdownMin = Center - 0.5f * Dim;

	ui_dropdown* Dropdown = &SceneState->ResolutionDropdown;
	InitDropdown(
		UiContext,
		Dropdown,
		MakeTrackedRectangle(GameState, DropdownMin, Dim),
		&GameState->TransientArena,
		ResolutionOptionCount
	);
	for(
		uint32_t ResolutionIndex = 0;
		ResolutionIndex < ResolutionOptionCount;
		ResolutionIndex++
	)
	{
		AddDropdownEntry(GameState, UiContext, Dropdown);
	}
}

void UpdateAndRenderOptionsMenu(
	game_state* GameState,
	options_menu_state* SceneState,
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

			ui_dropdown* Dropdown = &SceneState->ResolutionDropdown;
			button_handle_event_result DropdownResult = ButtonHandleEvent(
				&SceneState->UiContext,
				&Dropdown->DropButton,
				MouseEvent,
				MouseEventWorldPos
			);
			if(DropdownResult == ButtonHandleEvent_TakeAction)
			{
				Dropdown->DropdownVisible = !Dropdown->DropdownVisible;
			}
			if(Dropdown->DropdownVisible)
			{
				uint32_t* ResolutionPairs = SceneState->ResolutionPairs;
				for(
					uint32_t ButtonIndex = 0;
					ButtonIndex < Dropdown->ButtonsMaxLen;
					ButtonIndex++
				)
				{
					ui_button* DropdownButton = (
						Dropdown->DropdownButtons + ButtonIndex
					);
					button_handle_event_result ButtonResult = ButtonHandleEvent(
						&SceneState->UiContext,
						DropdownButton,
						MouseEvent,
						MouseEventWorldPos
					);
					if(ButtonResult == ButtonHandleEvent_TakeAction)
					{
						SetWindowSize(
							GameState,
							WindowWidth,
							WindowHeight,
							ResolutionPairs[2 * ButtonIndex],
							ResolutionPairs[2 * ButtonIndex + 1]
						);
						Dropdown->DropdownVisible = false;
						break;
					}
				}
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

	PushClear(&GameState->RenderGroup, Vector4(0.25f, 0.25f, 0.25f, 1.0f));

	vector4 Black = Vector4(0.0f, 0.0f, 0.0f, 1.0f);

	ui_dropdown* Dropdown = &SceneState->ResolutionDropdown;
	PushButtonToRenderGroup(
		*Dropdown->DropButton.Rectangle,
		BitmapHandle_TestCard2,
		&GameState->RenderGroup,
		&GameState->Assets, 
		"Set Resolution",
		sizeof("Set Resolution"),
		FontHandle_TestFont,
		Black, 
		&GameState->FrameArena
	);
	if(Dropdown->DropdownVisible)
	{
		uint32_t* ResolutionPairs = SceneState->ResolutionPairs;
		char CharBuffer[256];
		uint32_t CharBufferSize = sizeof(CharBuffer);
		for(
			uint32_t ButtonIndex = 0;
			ButtonIndex < Dropdown->DropdownButtonsLen;
			ButtonIndex++
		)
		{
			ui_button* DropdownButton = Dropdown->DropdownButtons + ButtonIndex;
			snprintf(
				CharBuffer,
				CharBufferSize,
				"%d x %d",
				ResolutionPairs[2 * ButtonIndex],
				ResolutionPairs[2 * ButtonIndex + 1]
			);
			PushButtonToRenderGroup(
				*DropdownButton->Rectangle,
				BitmapHandle_TestCard2,
				&GameState->RenderGroup,
				&GameState->Assets, 
				CharBuffer,
				CharBufferSize,
				FontHandle_TestFont,
				Black, 
				&GameState->FrameArena
			);
		}
	}
}