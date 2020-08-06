#include "apocalypse_deck_editor.h"
#include "apocalypse_platform.h"
#include "apocalypse.h"

void StartDeckEditor(game_state* GameState, game_offscreen_buffer* BackBuffer)
{
	ResetMemArena(&GameState->TransientArena);
	GameState->SceneState = PushStruct(
		&GameState->TransientArena, deck_editor_state
	);
	ResetAssets(&GameState->Assets);
	deck_editor_state* SceneState = (deck_editor_state*) GameState->SceneState;

	SceneState->Definitions = DefineCards(&GameState->TransientArena);
	card_definitions* Definitions = SceneState->Definitions;
	vector2 Dim = Vector2(60.0f, 90.0f);
	uint32_t NumRows = 2;
	uint32_t NumCols = 4;
	float XMargin = 10.0f;
	float YMargin = 10.0f;
	for(uint32_t Row = 0; Row < NumRows; Row++)
	{
		for(uint32_t Col = 0; Col < NumCols; Col++)
		{
			uint32_t Index = NumCols * Row + Col;
			collection_card* CollectionCard = (
				SceneState->CollectionCards + Index
			);
			CollectionCard->Rectangle = MakeRectangle(
				Vector2(
					(Dim.X + XMargin)* Col,
					(Dim.Y + YMargin) * (NumRows - Row - 1)
				),
				Dim
			);
			if(Index < Definitions->NumCards)
			{
				CollectionCard->Active = true;
				CollectionCard->Definition = &Definitions->Array[Index];
			}
			else
			{
				CollectionCard->Active = false;
			}
		}
	}

	SceneState->DeckNameBufferSize = 32;
	SceneState->DeckName = PushArray(
		&GameState->TransientArena, SceneState->DeckNameBufferSize, char
	);
	SceneState->DeckNameSet = false;

	{
		text_input* TextInput = &SceneState->DeckNameInput;
		*TextInput = {};
		ClearAllFlags(TextInput);
		SetFlag(TextInput, TextInput_Active);
		SetFlag(TextInput, TextInput_Selected);
		TextInput->CursorPos = 0;
		TextInput->FontHeight = 20.0f;
		TextInput->Rectangle = MakeRectangleCentered(
			Vector2(BackBuffer->Width / 2.0f, BackBuffer->Height / 2.0f),
			Vector2(BackBuffer->Width / 5.0f, TextInput->FontHeight)
		);
		TextInput->BufferSize = SceneState->DeckNameBufferSize;
		TextInput->Buffer = PushArray(
			&GameState->TransientArena, TextInput->BufferSize, char
		);
		TextInput->SubmitCallback = StandardSubmit;
		// TODO: maybe need to track submit callback data when initializing so 	
		// CONT: the updater can be abstracted
	}
}

void StartDeckEditorCallback(void* Data)
{
	game_state* GameState = (game_state*) Data;
	GameState->Scene = SceneType_DeckEditor;
}

void UpdateAndRenderDeckEditor(
	game_state* GameState,
	deck_editor_state* SceneState,
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
				if(
					CheckFlag(&SceneState->DeckNameInput, TextInput_Active) &&
					CheckFlag(&SceneState->DeckNameInput, TextInput_Selected)
				)
				{
					text_input* TextInput = &SceneState->DeckNameInput;
					switch(KeyboardEvent->Code)
					{
						// TODO: Handle backspace and delete
						case(0x08):
						{
							// NOTE: backspace
							// TODO: implement
							break;
						}
						case(0x09):
						{
							// NOTE: Tab
							// TODO: implement
							break;
						}
						case(0x10):
						{
							// NOTE: Shift
							// TODO: implement
							// TODO: handle someone pressing shift already when
							// CONT: selecting the text box
							if(KeyboardEvent->IsDown)
							{
								SetFlag(TextInput, TextInput_ShiftIsDown);
							}
							else
							{
								ClearFlag(TextInput, TextInput_ShiftIsDown);
							}
							break;
						}
						case(0x0D):
						{
							// NOTE: Return
							standard_submit_args SubmitArgs = {};
							SubmitArgs.TextInput = TextInput;
							SubmitArgs.DataLength = TextInput->CursorPos;
							SubmitArgs.Buffer = TextInput->Buffer;
							SubmitArgs.Dest = SceneState->DeckName;
							TextInput->SubmitCallback(&SubmitArgs);
							SceneState->DeckNameSet = true;
							break;
						}
						case(0x20):
						{
							// NOTE: Space
							// TODO: implement
							break;
						}
						case(0x25):
						{
							// NOTE: Left
							// TODO: implement							
							break;
						}
						case(0x26):
						{
							// NOTE: Up
							// TODO: implement							
							break;
						}
						case(0x27):
						{
							// NOTE: Right
							// TODO: implement							
							break;
						}
						case(0x28):
						{
							// NOTE: Down
							// TODO: implement
							break;
						}
						case(0x30):
						case(0x31):
						case(0x32):
						case(0x33):
						case(0x34):
						case(0x35):
						case(0x36):
						case(0x37):
						case(0x38):
						case(0x39):
						{
							// NOTE: Numbers
							break;
						}
						case(0x41):
						case(0x42):
						case(0x43):
						case(0x44):
						case(0x45):
						case(0x46):
						case(0x47):
						case(0x48):
						case(0x49):
						case(0x4A):
						case(0x4B):
						case(0x4C):
						case(0x4D):
						case(0x4E):
						case(0x4F):
						case(0x50):
						case(0x51):
						case(0x52):
						case(0x53):
						case(0x54):
						case(0x55):
						case(0x56):
						case(0x57):
						case(0x58):
						case(0x59):
						case(0x5A):
						case(0x5B):
						{
							// NOTE: Letters
							if(!KeyboardEvent->IsDown)
							{
								char Letter;
								if(!CheckFlag(TextInput, TextInput_ShiftIsDown))
								{
									Letter = KeyboardEvent->Code + 0x20; 
								}
								else
								{
									Letter = KeyboardEvent->Code;
								}
								TextInput->Buffer[TextInput->CursorPos] = Letter;
								TextInput->CursorPos++;
								if(
									TextInput->CursorPos >= 
									TextInput->BufferSize
								)
								{
									TextInput->CursorPos = (
										TextInput->BufferSize - 1
									);
								}
								TextInput->Buffer[TextInput->CursorPos] = 0;
							}
							// TODO: set a flag for press and hold
							// TODO: define a rate for press and hold
							break;
						}
					}
				}
			}

			UserEventIndex++;
		}
	}

	PushClear(&GameState->RenderGroup, Vector4(0.25f, 0.25f, 0.25f, 1.0f));
	for(
		uint32_t Index = 0;
		Index < ARRAY_COUNT(SceneState->CollectionCards);
		Index++
	)
	{
		collection_card* Card = SceneState->CollectionCards + Index;
		if(Card->Active)
		{
			PushSizedBitmap(
				&GameState->RenderGroup,
				&GameState->Assets,
				BitmapHandle_TestCard2,
				GetCenter(Card->Rectangle),
				Vector2(Card->Rectangle.Dim.X, 0.0f),
				Vector2(0.0f, Card->Rectangle.Dim.Y),
				Vector4(1.0f, 1.0f, 1.0f, 1.0f)
			);
		}
	}
	if(CheckFlag(&SceneState->DeckNameInput, TextInput_Active))
	{
		text_input* TextInput = &SceneState->DeckNameInput;
		vector2 TopLeft = GetTopLeft(TextInput->Rectangle);
		if(TextInput->Buffer[0] != 0)
		{
			PushTextTopLeft(
				&GameState->RenderGroup,
				&GameState->Assets,
				FontHandle_TestFont,
				TextInput->Buffer,
				TextInput->BufferSize,
				TextInput->FontHeight,
				TopLeft,
				Vector4(1.0f, 1.0f, 1.0f, 1.0f),
				&GameState->FrameArena
			);
		}
	}
	if(SceneState->DeckNameSet)
	{
		PushTextTopLeft(
			&GameState->RenderGroup,
			&GameState->Assets,
			FontHandle_TestFont,
			SceneState->DeckName,
			SceneState->DeckNameBufferSize,
			20.0f,
			Vector2(BackBuffer->Width - 50.0f, 50.0f),
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			&GameState->FrameArena
		);
	}
}