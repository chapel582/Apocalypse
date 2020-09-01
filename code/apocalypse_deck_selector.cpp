#include "apocalypse_deck_selector.h"
#include "apocalypse_deck_editor.h"
#include "apocalypse_card_game.h"
#include "apocalypse_platform.h"
#include "apocalypse.h"
#include "apocalypse_deck_storage.h"
#include "apocalypse_render_group.h"
#include "apocalypse_assets.h"

void DeckSelectorPrepNextScene(
	game_state* GameState, deck_selector_state* SceneState, bool AlreadyExists
)
{
	switch(SceneState->ToStart)
	{
		case(SceneType_CardGame):
		{
			if(AlreadyExists)
			{
				StartCardGamePrep(GameState, SceneState->DeckName, "P2Deck");
			}
			else
			{
				DisplayMessageFor(
					GameState,
					&SceneState->Alert,
					"Cannot load non-existent deck",
					1.0f
				);
			}
			break;
		}
		case(SceneType_DeckEditor):
		{
			StartDeckEditorPrep(GameState, SceneState->DeckName, AlreadyExists);
			break;
		}
		default:
		{
			ASSERT(false);
			break;
		}
	}
}

void StartDeckSelectorPrep(game_state* GameState, scene_type ToStart)
{
	start_deck_selector_args* SceneArgs = PushStruct(
		&GameState->TransientArena, start_deck_selector_args
	);

	SceneArgs->ToStart = ToStart;

	GameState->SceneArgs = SceneArgs;
	GameState->Scene = SceneType_DeckSelector; 
}

void StartDeckSelector(game_state* GameState, game_offscreen_buffer* BackBuffer)
{
	start_deck_selector_args* Args = (start_deck_selector_args*) (
		GameState->SceneArgs
	);
	scene_type ToStart = Args->ToStart;

	ResetMemArena(&GameState->TransientArena);
	
	GameState->SceneState = PushStruct(
		&GameState->TransientArena, deck_selector_state
	);
	ResetAssets(&GameState->Assets);
	deck_selector_state* SceneState = (deck_selector_state*) (
		GameState->SceneState
	);
	SceneState->ToStart = ToStart;

	InitUiContext(&SceneState->UiContext);
	ui_context* UiContext = &SceneState->UiContext;

	SceneState->DeckNameBufferSize = PLATFORM_MAX_PATH;
	SceneState->DeckName = PushArray(
		&GameState->TransientArena, SceneState->DeckNameBufferSize, char
	);
	memset(
		SceneState->DeckName, 0, SceneState->DeckNameBufferSize * sizeof(char)
	);

	InitTextInput(
		UiContext,
		&SceneState->DeckNameInput,
		20.0f,
		SceneState->DeckName,
		SceneState->DeckNameBufferSize
	);
	SceneState->DeckNameInputRectangle = MakeRectangleCentered(
		Vector2(BackBuffer->Width / 2.0f, BackBuffer->Height / 2.0f),
		Vector2(BackBuffer->Width / 5.0f, SceneState->DeckNameInput.FontHeight)
	);
	SetActive(UiContext, SceneState->DeckNameInput.UiId);

	SceneState->DeckNamesSize = PLATFORM_MAX_PATH * MAX_DECKS_SAVED;
	SceneState->DeckNames = PushArray(
		&GameState->TransientArena,
		SceneState->DeckNamesSize,
		char
	);
	memset(SceneState->DeckNames, 0, SceneState->DeckNamesSize);
	GetAllDeckPaths(SceneState->DeckNames, SceneState->DeckNamesSize);
	SceneState->LoadDeckButtonDim = Vector2(160.0f, 30.0f);
	SceneState->LoadDeckButtonsYStart = (float) BackBuffer->Height;
	SceneState->LoadDeckButtonsYMargin = 0.1f * SceneState->LoadDeckButtonDim.Y;
	load_deck_button* LoadDeckButtons = SceneState->LoadDeckButtons;
	for(
		uint32_t ButtonIndex = 0;
		ButtonIndex < ARRAY_COUNT(SceneState->LoadDeckButtons);
		ButtonIndex++
	)
	{
		load_deck_button* Button = LoadDeckButtons + ButtonIndex;
		Button->UiId = GetId(UiContext);
	}

	SceneState->Alert = MakeAlert();
}

void UpdateAndRenderDeckSelector(
	game_state* GameState,
	deck_selector_state* SceneState,
	game_offscreen_buffer* BackBuffer,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
)
{
	user_event_index UserEventIndex = 0;
	int MouseEventIndex = 0;
	int KeyboardEventIndex = 0;

	ui_context* UiContext = &SceneState->UiContext;
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
			
			rectangle DeckNameInputRectangle = (
				SceneState->DeckNameInputRectangle
			);
			TextInputHandleMouse(
				UiContext,
				&SceneState->DeckNameInput,
				DeckNameInputRectangle,
				MouseEvent,
				MouseEventWorldPos
			);

			vector2 Dim = SceneState->LoadDeckButtonDim;
			load_deck_button* LoadDeckButtons = SceneState->LoadDeckButtons;
			char* CurrentDeckName = SceneState->DeckNames;
			flat_string_array_reader FlatArrayReader;
			InitFlatStringArrayReader(
				&FlatArrayReader,
				CurrentDeckName,
				SceneState->DeckNamesSize
			);
			for(
				uint32_t ButtonIndex = 0;
				ButtonIndex < ARRAY_COUNT(SceneState->LoadDeckButtons);
				ButtonIndex++
			)
			{
				if(CurrentDeckName == NULL)
				{
					break;
				}
				load_deck_button* LoadDeckButton = (
					LoadDeckButtons + ButtonIndex
				);
				rectangle Rectangle = MakeNextRectangle(
					GetRight(DeckNameInputRectangle) + 10.0f,
					SceneState->LoadDeckButtonsYStart,
					SceneState->LoadDeckButtonsYMargin,
					SceneState->LoadDeckButtonDim,
					ButtonIndex
				);

				button_handle_event_result Result = ButtonHandleEvent(
					UiContext,
					LoadDeckButton->UiId,
					Rectangle,
					MouseEvent,
					MouseEventWorldPos
				);
				if(Result == ButtonHandleEvent_TakeAction)
				{
					uint32_t DotIndex = FindIndex(
						CurrentDeckName, '.', FlatArrayReader.BytesRemaining
					);
					strcpy_s(
						SceneState->DeckName,
						SceneState->DeckNameBufferSize,
						CurrentDeckName
					);
					SceneState->DeckName[DotIndex] = 0;

					DeckSelectorPrepNextScene(GameState, SceneState, true);
					break;
				}
				else
				{
					CurrentDeckName = GetNextString(&FlatArrayReader); 
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
				// NOTE: :common keyboard actions across all states
				switch(KeyboardEvent->Code)
				{
					case(0x1B): // NOTE: Escape V-code
					{
						GameState->Scene = SceneType_MainMenu; 
						break;
					}
				}

				text_input_kb_result KeyboardResult = (
					TextInputHandleKeyboard(
						UiContext, &SceneState->DeckNameInput, KeyboardEvent
					)
				);
				if(KeyboardResult == TextInputKbResult_Submit)
				{
					bool AlreadyExists = false;
					char* CurrentDeckName = SceneState->DeckNames;
					flat_string_array_reader FlatArrayReader;
					InitFlatStringArrayReader(
						&FlatArrayReader,
						CurrentDeckName,
						SceneState->DeckNamesSize
					);
					for(
						uint32_t ButtonIndex = 0;
						(
							ButtonIndex < 
							ARRAY_COUNT(SceneState->LoadDeckButtons)
						);
						ButtonIndex++
					)
					{
						if(CurrentDeckName == NULL)
						{
							break;
						}

						uint32_t StopAt = FindIndex(
							CurrentDeckName, 
							'.', 
							FlatArrayReader.BytesRemaining
						);

						if(
							StrCmp(								
								CurrentDeckName,
								SceneState->DeckName,
								StopAt
							)
						)
						{
							// NOTE: the name the user typed in is already 
							// CONT: a deck. load that deck
							AlreadyExists = true;
							break;
						}

						CurrentDeckName = GetNextString(&FlatArrayReader);
					}
					DeckSelectorPrepNextScene(
						GameState, SceneState, AlreadyExists
					);
				}
			}

			UserEventIndex++;
		}
	}

	UpdateTextInput(UiContext, &SceneState->DeckNameInput, DtForFrame);

	render_group* DefaultRenderGroup = &GameState->RenderGroup;
	assets* Assets = &GameState->Assets;
	PushClear(DefaultRenderGroup, Vector4(0.25f, 0.25f, 0.25f, 1.0f));
	
	vector4 White = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	vector4 Black = Vector4(0.0f, 0.0f, 0.0f, 1.0f);

	vector4 FontColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	vector4 BackgroundColor = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
	bitmap_handle Background = BitmapHandle_TestCard2;
	rectangle DeckNameInputRectangle = SceneState->DeckNameInputRectangle;
	PushTextInput(
		UiContext,
		&SceneState->DeckNameInput,
		FontColor,
		DeckNameInputRectangle,
		Background,
		BackgroundColor,
		Assets,
		DefaultRenderGroup,
		&GameState->FrameArena
	);

	vector2 Dim = SceneState->LoadDeckButtonDim;
	load_deck_button* LoadDeckButtons = SceneState->LoadDeckButtons;
	flat_string_array_reader FlatArrayReader;
	char* CurrentDeckName = SceneState->DeckNames;
	InitFlatStringArrayReader(
		&FlatArrayReader, CurrentDeckName, SceneState->DeckNamesSize
	);
	for(
		uint32_t ButtonIndex = 0;
		ButtonIndex < ARRAY_COUNT(SceneState->LoadDeckButtons);
		ButtonIndex++
	)
	{
		if(CurrentDeckName == NULL)
		{
			break;
		}
		load_deck_button* LoadDeckButton = (
			LoadDeckButtons + ButtonIndex
		);
		rectangle Rectangle = MakeNextRectangle(
			GetRight(DeckNameInputRectangle) + 10.0f,
			SceneState->LoadDeckButtonsYStart,
			SceneState->LoadDeckButtonsYMargin,
			SceneState->LoadDeckButtonDim,
			ButtonIndex
		);

		uint32_t StopAt = FindIndex(
			CurrentDeckName, '.', FlatArrayReader.BytesRemaining
		);
		PushButtonToRenderGroup(
			Rectangle,
			BitmapHandle_TestCard2,
			DefaultRenderGroup,
			Assets, 
			CurrentDeckName,
			StopAt,
			FontHandle_TestFont,
			Black,
			&GameState->FrameArena
		);

		CurrentDeckName = GetNextString(&FlatArrayReader);
	}

	PushCenteredAlert(&SceneState->Alert, GameState, BackBuffer);
}