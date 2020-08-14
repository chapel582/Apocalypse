#include "apocalypse_deck_editor.h"
#include "apocalypse_platform.h"
#include "apocalypse.h"
#include "apocalypse_info_card.h"
#include "apocalypse_card_definitions.h"
#include "apocalypse_deck_storage.h"
#include "apocalypse_render_group.h"

void AddLetterToTextInput(text_input* TextInput)
{
	TextInput->Buffer[TextInput->CursorPos] = TextInput->CharDown;
	TextInput->CursorPos++;
	if(TextInput->CursorPos >= TextInput->BufferSize)
	{
		TextInput->CursorPos = TextInput->BufferSize - 1;
	}
	TextInput->Buffer[TextInput->CursorPos] = 0;
}

void Backspace(text_input* TextInput)
{
	if(TextInput->CursorPos > 0)
	{
		TextInput->CursorPos--;	
	}
	TextInput->Buffer[TextInput->CursorPos] = 0;
}

void PressAndHoldKeyboardEvent(
	text_input* TextInput,
	text_input_repeat_callback* RepeatCallback,
	game_keyboard_event* KeyboardEvent,
	char Character
)
{
	if(!KeyboardEvent->IsDown)
	{
		ClearFlag(TextInput, TextInput_CharDownDelay);
		ClearFlag(TextInput, TextInput_CharDown);

		TextInput->CursorColor.A = 1.0f;
		TextInput->CursorAlphaState = CursorAlphaState_Decreasing; 
	}
	else
	{
		TextInput->CursorColor.A = 1.0f;
		TextInput->CursorAlphaState = CursorAlphaState_Static;

		TextInput->CharDown = Character;
		TextInput->RepeatTimer = 0.0f;
		TextInput->RepeatCallback = RepeatCallback;
		RepeatCallback(TextInput);
		SetFlag(TextInput, TextInput_CharDownDelay);
	}
}

void PushCursor(
	text_input* TextInput, render_group* RenderGroup, vector2 Offset
)
{
	if(CheckFlag(TextInput, TextInput_Selected))
	{
		PushRect(
			RenderGroup,
			MakeRectangle(
				TextInput->Rectangle.Min + Offset,
				Vector2(2.0f, TextInput->FontHeight)
			),
			TextInput->CursorColor
		);
	}
}

struct submit_deck_name_args
{
	standard_submit_args StandardArgs;
	deck_editor_state* SceneState;
};

void SubmitDeckName(void* Data)
{
	submit_deck_name_args* Args = (submit_deck_name_args*) Data; 
	StandardSubmit(&Args->StandardArgs);
	deck_editor_state* SceneState = Args->SceneState;
	SceneState->DeckNameSet = true;

	uint32_t VisibleInteractable = (
		UiButton_Interactable | UiButton_Visible
	);
	AddButtonsFlags(
		SceneState->StaticButtons,
		ARRAY_COUNT(SceneState->StaticButtons),
		VisibleInteractable
	);
	AddButtonsFlags(
		SceneState->CollectionButtons,
		ARRAY_COUNT(SceneState->CollectionButtons),
		VisibleInteractable
	);
}

struct save_deck_button_args
{
	game_state* GameState;
	alert* Alert;
	uint32_t* CardCount;
	deck_editor_card* DeckCards;
	char* DeckName;
};

void SaveDeckButtonCallback(void* Data)
{
	save_deck_button_args* Args = (save_deck_button_args*) Data;
	loaded_deck Deck = {};
	game_state* GameState = Args->GameState;
	alert* Alert = Args->Alert;
	uint32_t CardCount = *Args->CardCount;
	deck_editor_card* Cards = Args->DeckCards;
	uint8_t IdIndex = 0;
	for(uint32_t CardIndex = 0; CardIndex < CardCount; CardIndex++)
	{
		deck_editor_card* Card = Cards + CardIndex;
		for(uint32_t Added = 0; Added < Card->Count; Added++)
		{
			Deck.Ids[IdIndex++] = Card->Definition->Id;
		}
	}
	ASSERT(IdIndex < 0xFF);
	Deck.Header.CardCount = IdIndex;

	char PathToDeck[256];
	GetDeckPath(PathToDeck, sizeof(PathToDeck), Args->DeckName);
	SaveDeck(PathToDeck, &Deck);
	DisplayMessageFor(GameState, Alert, "Saved Deck", 1.0f);
}

void RemoveCardFromDeck(
	deck_editor_cards* DeckCards, deck_editor_card* DeckCard
)
{
	ASSERT(DeckCards->CardsInDeck != 0);

	DeckCard->Count--;
	DeckCards->CardsInDeck--;

	if(DeckCard->Count > 0)
	{
		if(DeckCard->Count == 1)
		{
			snprintf(
				DeckCard->Button->Text,
				ARRAY_COUNT(DeckCard->Button->Text), 
				"%d",
				DeckCard->Definition->Id
			);
		}
		else
		{
			snprintf(
				DeckCard->Button->Text,
				ARRAY_COUNT(DeckCard->Button->Text), 
				"%d x %d",
				DeckCard->Definition->Id,
				DeckCard->Count
			);
		}
	}
	else
	{
		DeckCards->ActiveCardCount--;

		DeckCard->Definition = NULL;
		RemoveButton(DeckCard->Button);

		vector2 Dim = DeckCards->Dim;
		uint32_t ActiveCards = 0;
		for(int Index = 0; Index < MAX_CARDS_IN_DECK; Index++)
		{
			DeckCard = DeckCards->Cards + Index;
			if(IsActive(DeckCard))
			{
				rectangle Rectangle = MakeRectangle(
					Vector2(
						DeckCards->XPos,
						(
							DeckCards->YStart - 
							(Dim.Y + DeckCards->YMargin) * 
							(ActiveCards + 1)
						)
					),
					Dim
				);

				DeckCard->Button->Rectangle = Rectangle;

				ActiveCards++;
			}
		}
	}
}

void RemoveCardFromDeckCallback(void* Data)
{
	remove_card_from_deck_args* Args = (remove_card_from_deck_args*) Data;
	RemoveCardFromDeck(Args->DeckCards, Args->DeckCard);
}

void AddCardToDeck(
	game_state* GameState,
	deck_editor_state* SceneState,
	deck_editor_cards* DeckCards,
	card_definition* Definition
)
{
	if(DeckCards->CardsInDeck >= MAX_CARDS_IN_DECK)
	{
		DisplayMessageFor(
			GameState,
			&SceneState->Alert,
			"Cannot add card to deck. Too many cards.",
			1.0f
		);
		goto end;
	}

	deck_editor_card* DeckCard;
	bool FoundCard = false;
	for(int Index = 0; Index < MAX_CARDS_IN_DECK; Index++)
	{
		DeckCard = DeckCards->Cards + Index;
		if(IsActive(DeckCard) && DeckCard->Definition == Definition)
		{
			DeckCard->Count++;
			snprintf(
				DeckCard->Button->Text,
				ARRAY_COUNT(DeckCard->Button->Text), 
				"%d x %d",
				Definition->Id,
				DeckCard->Count
			);
			FoundCard = true;
			break;
		}
	}
	if(FoundCard)
	{
		DeckCards->CardsInDeck++;
		goto end;
	}

	vector2 Dim = DeckCards->Dim;
	for(int Index = 0; Index < MAX_CARDS_IN_DECK; Index++)
	{
		DeckCard = DeckCards->Cards + Index;
		if(!IsActive(DeckCard))
		{
			DeckCard->Definition = Definition;
			rectangle Rectangle = MakeRectangle(
				Vector2(
					DeckCards->XPos,
					(
						DeckCards->YStart - 
						(Dim.Y + DeckCards->YMargin) * 
						(DeckCards->ActiveCardCount + 1)
					)
				),
				Dim
			);

			DeckCard->Count++;

			DeckCard->RemoveCardFromDeckArgs.DeckCards = DeckCards;
			DeckCard->RemoveCardFromDeckArgs.DeckCard = DeckCard;

			DeckCard->Button = AddButton(
				SceneState->DeckButtons,
				ARRAY_COUNT(SceneState->DeckButtons),
				Rectangle,
				BitmapHandle_TestCard2,
				FontHandle_TestFont,
				NULL,
				Vector4(0.0f, 0.0f, 0.0f, 1.0f),
				RemoveCardFromDeckCallback,
				&DeckCard->RemoveCardFromDeckArgs
			);
			snprintf(
				DeckCard->Button->Text,
				ARRAY_COUNT(DeckCard->Button->Text),
				"%d",
				Definition->Id
			);
			SetFlags(
				DeckCard->Button, UiButton_Visible | UiButton_Interactable
			);
			break;
		}
	}
	
	DeckCards->ActiveCardCount++;
	DeckCards->CardsInDeck++;
	goto end;
	
end:
	return;
}

struct add_card_to_deck_args
{
	game_state* GameState;
	deck_editor_state* SceneState;
	deck_editor_cards* DeckCards;
	card_definition* Definition;
};

void AddCardToDeckCallback(void* Data)
{
	add_card_to_deck_args* Args = (add_card_to_deck_args*) Data;
	AddCardToDeck(
		Args->GameState, Args->SceneState, Args->DeckCards, Args->Definition
	);
}

// TODO: remove card from deck

void SetCollectionCardDefinitions(
	collection_card* CollectionCards,
	ui_button* CollectionButtons,
	card_definitions* Definitions,
	uint32_t CollectionStartIndex,
	uint32_t NumRows,
	uint32_t NumCols
)
{
	for(uint32_t Row = 0; Row < NumRows; Row++)
	{
		for(uint32_t Col = 0; Col < NumCols; Col++)
		{
			uint32_t BrowseIndex = NumCols * Row + Col;
			collection_card* CollectionCard = CollectionCards + BrowseIndex;
			uint32_t CollectionIndex = CollectionStartIndex + BrowseIndex;
			if(CollectionIndex < Definitions->NumCards)
			{
				CollectionCard->Definition = (
					Definitions->Array + CollectionIndex
				);
				ui_button* Button = CollectionCard->Button;
				add_card_to_deck_args* Data = (
					(add_card_to_deck_args*) Button->Data
				);
				Data->Definition = Definitions->Array + CollectionIndex;
			}
			else
			{
				CollectionCard->Definition = NULL;
			}
		}
	}
}
void CollectionCardsNext(void* Data)
{
	deck_editor_state* SceneState = (deck_editor_state*) Data;
	uint32_t OldStartIndex = SceneState->CollectionStartIndex;
	uint32_t NumRows = SceneState->NumRows;
	uint32_t NumCols = SceneState->NumCols;
	card_definitions* Definitions = SceneState->Definitions;

	SceneState->CollectionStartIndex += NumRows * NumCols;
	if(SceneState->CollectionStartIndex >= ((int32_t) Definitions->NumCards))
	{
		SceneState->CollectionStartIndex = OldStartIndex;
	}
	else
	{
		SetCollectionCardDefinitions(
			SceneState->CollectionCards,
			SceneState->CollectionButtons,
			Definitions,
			SceneState->CollectionStartIndex,
			NumRows,
			NumCols
		);
	}
}

void CollectionCardsPrev(void* Data)
{
	deck_editor_state* SceneState = (deck_editor_state*) Data;

	uint32_t OldStartIndex = SceneState->CollectionStartIndex;
	uint32_t NumRows = SceneState->NumRows;
	uint32_t NumCols = SceneState->NumCols;
	card_definitions* Definitions = SceneState->Definitions;

	SceneState->CollectionStartIndex -= NumRows * NumCols;
	if(SceneState->CollectionStartIndex < 0)
	{
		SceneState->CollectionStartIndex = 0;
	}
	else
	{
		SetCollectionCardDefinitions(
			SceneState->CollectionCards,
			SceneState->CollectionButtons,
			Definitions,
			SceneState->CollectionStartIndex,
			NumRows,
			NumCols
		);
	}
}

void StartDeckEditor(game_state* GameState, game_offscreen_buffer* BackBuffer)
{
	ResetMemArena(&GameState->TransientArena);
	GameState->SceneState = PushStruct(
		&GameState->TransientArena, deck_editor_state
	);
	ResetAssets(&GameState->Assets);
	deck_editor_state* SceneState = (deck_editor_state*) GameState->SceneState;
	InitButtons(
		SceneState->CollectionButtons, 
		ARRAY_COUNT(SceneState->CollectionButtons)
	);
	InitButtons(
		SceneState->DeckButtons, 
		ARRAY_COUNT(SceneState->DeckButtons)
	);
	InitButtons(
		SceneState->StaticButtons, 
		ARRAY_COUNT(SceneState->StaticButtons)
	);
	
	deck_editor_cards* DeckCards = &SceneState->DeckCards;
	*DeckCards = {};
	DeckCards->Dim = Vector2(90.0f, 30.0f);
	DeckCards->XPos = (
		BackBuffer->Width - DeckCards->Dim.X
	);
	DeckCards->YStart = (float) BackBuffer->Height;
	DeckCards->YMargin = 0.1f * DeckCards->Dim.Y;
	memset(
		DeckCards->Cards,
		0,
		ARRAY_COUNT(DeckCards->Cards) * sizeof(deck_editor_card)
	);

	SceneState->Definitions = DefineCards(&GameState->TransientArena);
	card_definitions* Definitions = SceneState->Definitions;
	SceneState->CollectionCardDim = Vector2(60.0f, 90.0f);
	vector2 Dim = SceneState->CollectionCardDim;
	SceneState->NumRows = 2;
	SceneState->NumCols = 4;
	uint32_t NumRows = SceneState->NumRows;
	uint32_t NumCols = SceneState->NumCols;
	float XOffset = 50.0f;
	float YOffset = 10.0f;
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
			if(Index < Definitions->NumCards)
			{
				rectangle Rectangle = MakeRectangle(
					Vector2(
						(Dim.X + XMargin) * Col + XOffset,
						(Dim.Y + YMargin) * (NumRows - Row - 1) + YOffset
					),
					Dim
				);
				CollectionCard->Definition = &Definitions->Array[Index];
				add_card_to_deck_args* Data = PushStruct(
					&GameState->TransientArena, add_card_to_deck_args
				);
				Data->GameState = GameState;
				Data->SceneState = SceneState;
				Data->DeckCards = &SceneState->DeckCards;
				Data->Definition = Definitions->Array + Index;
				CollectionCard->Button = AddButton(
					SceneState->CollectionButtons,
					ARRAY_COUNT(SceneState->CollectionButtons),
					Rectangle,
					BitmapHandle_TestCard2,
					FontHandle_TestFont,
					"",
					Vector4(0.0f, 0.0f, 0.0f, 1.0f),
					AddCardToDeckCallback,
					Data
				);
			}
			else
			{
				CollectionCard->Definition = NULL;
			}
		}
	}
	SceneState->CollectionStartIndex = 0;

	vector2 BrowseCollectionButtonDim = Vector2(
		XOffset - 2 * XMargin, NumRows * (YMargin + Dim.Y)
	);
	rectangle ScrollButtonRectangle = MakeRectangle(
		Vector2(XMargin, YOffset),
		BrowseCollectionButtonDim
	);
	AddButton(
		SceneState->StaticButtons,
		ARRAY_COUNT(SceneState->StaticButtons),
		ScrollButtonRectangle,
		BitmapHandle_TestCard2,
		FontHandle_TestFont,
		"",
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		CollectionCardsPrev,
		SceneState
	);

	ScrollButtonRectangle = MakeRectangle(
		Vector2(NumCols * (Dim.X + XMargin) + XOffset, YOffset),
		BrowseCollectionButtonDim
	);
	AddButton(
		SceneState->StaticButtons,
		ARRAY_COUNT(SceneState->StaticButtons),
		ScrollButtonRectangle,
		BitmapHandle_TestCard2,
		FontHandle_TestFont,
		"",
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		CollectionCardsNext,
		SceneState
	);

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
		TextInput->SubmitCallback = SubmitDeckName;
		// TODO: maybe need to track submit callback data when initializing so 	
		// CONT: the submission can be abstracted
		TextInput->RepeatDelay = 1.0f;
		TextInput->RepeatPeriod = 0.05f;
		TextInput->FontColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		TextInput->CursorColor = TextInput->FontColor;
		TextInput->CursorAlphaState = CursorAlphaState_Decreasing;
		TextInput->BackgroundColor = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
		TextInput->Background = BitmapHandle_TestCard2;
	}

	SceneState->Alert = Alert();

	vector2 SaveButtonDim = Vector2(
		1.5f * SceneState->DeckCards.Dim.X, SceneState->DeckCards.Dim.Y
	);
	rectangle SaveButtonRectangle = MakeRectangle(
		Vector2(SceneState->DeckCards.XPos - SaveButtonDim.X, 0.0f),
		SaveButtonDim
	);
	save_deck_button_args* SaveDeckButtonArgs = (
		PushStruct(&GameState->TransientArena, save_deck_button_args)
	);
	*SaveDeckButtonArgs = {};
	SaveDeckButtonArgs->GameState = GameState;
	SaveDeckButtonArgs->Alert = &SceneState->Alert;
	SaveDeckButtonArgs->DeckName = SceneState->DeckName;
	SaveDeckButtonArgs->DeckCards = SceneState->DeckCards.Cards;
	SaveDeckButtonArgs->CardCount = &SceneState->DeckCards.ActiveCardCount;
	AddButton(
		SceneState->StaticButtons,
		ARRAY_COUNT(SceneState->StaticButtons),
		SaveButtonRectangle,
		BitmapHandle_TestCard2,
		FontHandle_TestFont,
		"Save Deck",
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		SaveDeckButtonCallback,
		SaveDeckButtonArgs
	);

	SceneState->DeckNamePos = (
		GetTopLeft(SaveButtonRectangle) + Vector2(0.0f, 3.0f)
	); 

	SceneState->InfoCardCenter = Vector2(
		BackBuffer->Width / 2.0f, BackBuffer->Height / 2.0f
	);
	vector2 ScaledInfoCardDim = 0.33f * Vector2(600.0f, 900.0f);
	SceneState->InfoCardXBound = Vector2(ScaledInfoCardDim.X, 0.0f);
	SceneState->InfoCardYBound = Vector2(0.0f, ScaledInfoCardDim.Y);
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

			if(CheckFlag(&SceneState->DeckNameInput, TextInput_Active))
			{
				text_input* TextInput = &SceneState->DeckNameInput;
				if(PointInRectangle(MouseEventWorldPos, TextInput->Rectangle))
				{
					if(MouseEvent->Type == PrimaryUp)
					{
						SetFlag(TextInput, TextInput_Selected);
					}
				}
				else
				{
					if(MouseEvent->Type == PrimaryUp)
					{
						ClearFlag(TextInput, TextInput_Selected);
						ClearFlag(TextInput, TextInput_CharDownDelay);
						ClearFlag(TextInput, TextInput_CharDown);
						ClearFlag(TextInput, TextInput_ShiftIsDown);
					}
				}
			}

			ButtonsHandleMouseEvent(
				SceneState->CollectionButtons,
				ARRAY_COUNT(SceneState->CollectionButtons),
				MouseEvent,
				MouseEventWorldPos
			);
			ButtonsHandleMouseEvent(
				SceneState->DeckButtons,
				ARRAY_COUNT(SceneState->DeckButtons),
				MouseEvent,
				MouseEventWorldPos
			);
			ButtonsHandleMouseEvent(
				SceneState->StaticButtons,
				ARRAY_COUNT(SceneState->StaticButtons),
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
							PressAndHoldKeyboardEvent(
								TextInput,
								Backspace,
								KeyboardEvent,
								KeyboardEvent->Code
							);
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
							submit_deck_name_args SubmitArgs = {};
							SubmitArgs.SceneState = SceneState;
							standard_submit_args* StandardArgs = (
								&SubmitArgs.StandardArgs
							);
							StandardArgs->TextInput = TextInput;
							StandardArgs->DataLength = TextInput->CursorPos;
							StandardArgs->Buffer = TextInput->Buffer;
							StandardArgs->Dest = SceneState->DeckName;
							TextInput->SubmitCallback(&SubmitArgs);
							break;
						}
						case(0x20):
						{
							// NOTE: Space
							PressAndHoldKeyboardEvent(
								TextInput,
								AddLetterToTextInput,
								KeyboardEvent,
								KeyboardEvent->Code
							);
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
							PressAndHoldKeyboardEvent(
								TextInput,
								AddLetterToTextInput,
								KeyboardEvent,
								KeyboardEvent->Code
							);
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
							char Letter;
							if(!CheckFlag(TextInput, TextInput_ShiftIsDown))
							{
								Letter = KeyboardEvent->Code + 0x20; 
							}
							else
							{
								Letter = KeyboardEvent->Code;
							}
							PressAndHoldKeyboardEvent(
								TextInput,
								AddLetterToTextInput,
								KeyboardEvent,
								Letter
							);
							break;
						}
						case(0x7F):
						{
							// NOTE: Delete
							// TODO: implement
							break;
						}
					}
				}
			}

			UserEventIndex++;
		}
	}

	// SECTION START: Update text input
	{
		text_input* TextInput = &SceneState->DeckNameInput;

		float Period = 1.0f;
		if(TextInput->CursorAlphaState == CursorAlphaState_Increasing)
		{
			TextInput->CursorColor.A += DtForFrame / Period;
			if(TextInput->CursorColor.A >= 1.0f)
			{
				TextInput->CursorColor.A = 1.0f;
				TextInput->CursorAlphaState = CursorAlphaState_Decreasing;
			}
		}
		else if(TextInput->CursorAlphaState == CursorAlphaState_Decreasing)
		{
			TextInput->CursorColor.A -= DtForFrame / Period;
			if(TextInput->CursorColor.A <= 0.0f)
			{
				TextInput->CursorColor.A = 0.0f;
				TextInput->CursorAlphaState = CursorAlphaState_Increasing;
			}
		}

		if(CheckFlag(TextInput, TextInput_CharDownDelay))
		{
			if(TextInput->RepeatTimer >= TextInput->RepeatDelay)
			{
				TextInput->RepeatTimer = 0.0f;
				TextInput->RepeatCallback(TextInput);
				ClearFlag(TextInput, TextInput_CharDownDelay);
				SetFlag(TextInput, TextInput_CharDown);
			}
		}
		else if(CheckFlag(TextInput, TextInput_CharDown))
		{
			if(TextInput->RepeatTimer >= TextInput->RepeatPeriod)
			{
				TextInput->RepeatTimer = 0.0f;
				TextInput->RepeatCallback(TextInput);
			}	
		}
		TextInput->RepeatTimer += DtForFrame;
	}
	// SECTION STOP: Update text input

	PushClear(&GameState->RenderGroup, Vector4(0.25f, 0.25f, 0.25f, 1.0f));
	
	vector4 White = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	for(
		uint32_t CollectionCardIndex = 0;
		CollectionCardIndex < ARRAY_COUNT(SceneState->CollectionCards);
		CollectionCardIndex++
	)
	{
		collection_card* CollectionCard = (
			SceneState->CollectionCards + CollectionCardIndex
		);
		if(IsActive(CollectionCard))
		{
			ui_button* Button = CollectionCard->Button;
			PushButtonToRenderGroup(
				Button,
				&GameState->RenderGroup,
				&GameState->Assets,
				&GameState->FrameArena, 
				White
			);
			if(CheckFlag(Button, UiButton_HoveredOver))
			{
				card_definition* Definition = CollectionCard->Definition;
				PushInfoCard(
					&GameState->RenderGroup,
					&GameState->Assets,
					SceneState->InfoCardCenter,
					SceneState->InfoCardXBound,
					SceneState->InfoCardYBound,
					White,
					&GameState->FrameArena,
					Definition->Attack,
					Definition->Health,
					Definition->PlayDelta,
					Definition->TapDelta,
					-1
				);
			}
		}
	}
	for(
		uint32_t DeckCardIndex = 0;
		DeckCardIndex < MAX_CARDS_IN_DECK;
		DeckCardIndex++
	)
	{
		deck_editor_card* DeckCard = (
			SceneState->DeckCards.Cards + DeckCardIndex
		);
		if(IsActive(DeckCard))
		{
			ui_button* Button = DeckCard->Button;
			PushButtonToRenderGroup(
				Button,
				&GameState->RenderGroup,
				&GameState->Assets,
				&GameState->FrameArena, 
				White
			);
			if(
				CheckFlag(Button, UiButton_Interactable) && 
				CheckFlag(Button, UiButton_HoveredOver)
			)
			{
				card_definition* Definition = DeckCard->Definition;
				PushInfoCard(
					&GameState->RenderGroup,
					&GameState->Assets,
					SceneState->InfoCardCenter,
					SceneState->InfoCardXBound,
					SceneState->InfoCardYBound,
					White,
					&GameState->FrameArena,
					Definition->Attack,
					Definition->Health,
					Definition->PlayDelta,
					Definition->TapDelta,
					-1
				);
			}
		}
	}
	PushButtonsToRenderGroup(
		SceneState->StaticButtons,
		ARRAY_COUNT(SceneState->StaticButtons),
		&GameState->RenderGroup,
		&GameState->Assets, 
		&GameState->FrameArena
	);

	if(CheckFlag(&SceneState->DeckNameInput, TextInput_Active))
	{
		text_input* TextInput = &SceneState->DeckNameInput;
		render_group* RenderGroup = &GameState->RenderGroup;
		PushSizedBitmap(
			RenderGroup,
			&GameState->Assets,
			TextInput->Background,
			GetCenter(TextInput->Rectangle),
			TextInput->Rectangle.Dim,				
			TextInput->BackgroundColor
		);
		push_text_result PushTextResult = {};

		if(TextInput->Buffer[0] != 0)
		{		
			vector2 TopLeft = GetTopLeft(TextInput->Rectangle);	
			PushTextResult = PushTextTopLeft(
				RenderGroup,
				&GameState->Assets,
				FontHandle_TestFont,
				TextInput->Buffer,
				TextInput->BufferSize,
				TextInput->FontHeight,
				TopLeft,
				TextInput->FontColor,
				&GameState->FrameArena
			);
			if(PushTextResult.Code == PushText_Success)
			{
				vector2 OffsetScreen = PushTextResult.Offset;
				vector2 OffsetWorld = TransformVectorToBasis(
					RenderGroup->CameraToScreen, OffsetScreen
				);
				vector2 OffsetCamera = TransformVectorToBasis(
					RenderGroup->WorldToCamera, OffsetWorld
				);

				PushCursor(TextInput, RenderGroup, OffsetCamera);
			}
		}
		if(PushTextResult.Code != PushText_Success)
		{
			// TODO: make the lpad more programmatic
			PushCursor(TextInput, RenderGroup, Vector2(2.0f, 0.0f));
		}
	}

	if(SceneState->DeckNameSet)
	{
		PushText(
			&GameState->RenderGroup,
			&GameState->Assets,
			FontHandle_TestFont,
			SceneState->DeckName,
			SceneState->DeckNameBufferSize,
			20.0f,
			SceneState->DeckNamePos,
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			&GameState->FrameArena
		);
	}

	PushCenteredAlert(&SceneState->Alert, GameState, BackBuffer);
}