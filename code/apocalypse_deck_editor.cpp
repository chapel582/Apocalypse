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
}

void SaveEditableDeck(
	game_state* GameState,
	alert* Alert,
	deck_editor_cards* DeckCards,
	char* DeckName
)
{
	loaded_deck Deck = {};
	uint8_t IdIndex = 0;
	deck_editor_card* Cards = DeckCards->Cards;
	for(
		uint32_t CardIndex = 0;
		CardIndex < ARRAY_COUNT(DeckCards->Cards);
		CardIndex++
	)
	{
		deck_editor_card* Card = Cards + CardIndex;
		if(IsActive(Card))
		{
			for(uint32_t Added = 0; Added < Card->Count; Added++)
			{
				Deck.Ids[IdIndex++] = Card->Definition->Id;
			}
		}
	}
	ASSERT(IdIndex < 0xFF);
	Deck.Header.CardCount = IdIndex;

	char PathToDeck[256];
	GetDeckPath(PathToDeck, sizeof(PathToDeck), DeckName);
	SaveDeck(PathToDeck, &Deck);
	DisplayMessageFor(GameState, Alert, "Saved Deck", 1.0f);
}

uint32_t GetCardsInDeck(deck_editor_cards* DeckCards)
{
	uint32_t CardsInDeck = 0;
	for(int Index = 0; Index < ARRAY_COUNT(DeckCards->Cards); Index++)
	{
		deck_editor_card* Card = DeckCards->Cards + Index;
		if(IsActive(Card))
		{
			CardsInDeck++;
		}
	}
	return CardsInDeck;
}

void RemoveCardFromDeck(
	deck_editor_cards* DeckCards, deck_editor_card* DeckCard
)
{
	DeckCard->Count--;

	if(DeckCard->Count <= 0)
	{
		DeckCard->Definition = NULL;
	}
}

void AddCardToDeck(
	game_state* GameState,
	deck_editor_state* SceneState,
	deck_editor_cards* DeckCards,
	card_definition* Definition
)
{
	if(GetCardsInDeck(DeckCards) >= MAX_CARDS_IN_DECK)
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
			FoundCard = true;
			break;
		}
	}
	if(FoundCard)
	{
		goto end;
	}

	vector2 Dim = DeckCards->Dim;
	for(int Index = 0; Index < MAX_CARDS_IN_DECK; Index++)
	{
		DeckCard = DeckCards->Cards + Index;
		if(!IsActive(DeckCard))
		{
			DeckCard->UiId = GetId(&SceneState->UiContext);
			DeckCard->Definition = Definition;
			DeckCard->Count++;
			break;
		}
	}	
	goto end;
	
end:
	return;
}

void CollectionCardsNext(deck_editor_state* SceneState)
{
	uint32_t OldStartIndex = SceneState->CollectionStartIndex;
	uint32_t NumRows = SceneState->NumRows;
	uint32_t NumCols = SceneState->NumCols;
	card_definitions* Definitions = SceneState->Definitions;

	SceneState->CollectionStartIndex += NumRows * NumCols;
	if(SceneState->CollectionStartIndex >= ((int32_t) Definitions->NumCards))
	{
		SceneState->CollectionStartIndex = OldStartIndex;
	}
}

void CollectionCardsPrev(deck_editor_state* SceneState)
{
	uint32_t OldStartIndex = SceneState->CollectionStartIndex;
	uint32_t NumRows = SceneState->NumRows;
	uint32_t NumCols = SceneState->NumCols;
	card_definitions* Definitions = SceneState->Definitions;

	SceneState->CollectionStartIndex -= NumRows * NumCols;
	if(SceneState->CollectionStartIndex < 0)
	{
		SceneState->CollectionStartIndex = 0;
	}
}

rectangle MakeDeckCardRectangle(
	deck_editor_cards* DeckCards, vector2 Dim, uint32_t ActiveCards
)
{
	return MakeRectangle(
		Vector2(
			DeckCards->XPos,
			DeckCards->YStart - (Dim.Y + DeckCards->YMargin) * (ActiveCards + 1)
		),
		Dim
	);
}

void StartDeckEditor(game_state* GameState, game_offscreen_buffer* BackBuffer)
{
	ResetMemArena(&GameState->TransientArena);
	GameState->SceneState = PushStruct(
		&GameState->TransientArena, deck_editor_state
	);
	ResetAssets(&GameState->Assets);
	deck_editor_state* SceneState = (deck_editor_state*) GameState->SceneState;
	InitUiContext(&SceneState->UiContext);
	
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
				InitButton(
					&SceneState->UiContext,
					&CollectionCard->Button,
					Rectangle
				);
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
	InitButton(
		&SceneState->UiContext,
		&SceneState->CollectionPrev,
		ScrollButtonRectangle
	);

	ScrollButtonRectangle = MakeRectangle(
		Vector2(NumCols * (Dim.X + XMargin) + XOffset, YOffset),
		BrowseCollectionButtonDim
	);
	InitButton(
		&SceneState->UiContext,
		&SceneState->CollectionNext,
		ScrollButtonRectangle
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

	SceneState->Alert = MakeAlert();

	vector2 SaveButtonDim = Vector2(
		1.5f * SceneState->DeckCards.Dim.X, SceneState->DeckCards.Dim.Y
	);
	rectangle SaveButtonRectangle = MakeRectangle(
		Vector2(SceneState->DeckCards.XPos - SaveButtonDim.X, 0.0f),
		SaveButtonDim
	);
	InitButton(
		&SceneState->UiContext,
		&SceneState->SaveButton, 
		SaveButtonRectangle
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

			ui_context* UiContext = &SceneState->UiContext;
			button_handle_event_result Result;
			for(
				uint32_t Index = 0;
				Index < ARRAY_COUNT(SceneState->CollectionCards);
				Index++
			)
			{
				collection_card* CollectionCard = (
					SceneState->CollectionCards + Index
				);

				uint32_t CollectionIndex = (
					SceneState->CollectionStartIndex + Index
				);
				if(CollectionIndex < SceneState->Definitions->NumCards)
				{
					Result = ButtonHandleEvent(
						UiContext,
						&CollectionCard->Button,
						MouseEvent,
						MouseEventWorldPos
					);
					if(Result == ButtonHandleEvent_TakeAction)
					{
						card_definition* Definition = (
							SceneState->Definitions->Array + CollectionIndex
						);
						AddCardToDeck(
							GameState,
							SceneState,
							&SceneState->DeckCards,
							Definition
						);
						break;
					}
				}
			}

			deck_editor_cards* DeckCards = &SceneState->DeckCards;
			vector2 Dim = DeckCards->Dim;
			uint32_t ActiveCards = 0;
			for(
				uint32_t Index = 0;
				Index < ARRAY_COUNT(DeckCards->Cards);
				Index++
			)
			{
				deck_editor_card* DeckCard = DeckCards->Cards + Index;
				if(IsActive(DeckCard))
				{
					rectangle Rectangle = MakeDeckCardRectangle(
						DeckCards, Dim, ActiveCards
					);

					Result = ButtonHandleEvent(
						UiContext,
						DeckCard->UiId,
						Rectangle,
						MouseEvent,
						MouseEventWorldPos
					);
					if(Result == ButtonHandleEvent_TakeAction)
					{
						RemoveCardFromDeck(DeckCards, DeckCard);
						break;
					}
					ActiveCards++;
				}
			}
			
			Result = ButtonHandleEvent(
				UiContext,
				&SceneState->SaveButton,
				MouseEvent,
				MouseEventWorldPos
			);
			if(Result == ButtonHandleEvent_TakeAction)
			{
				SaveEditableDeck(
					GameState,
					&SceneState->Alert,
					DeckCards,
					SceneState->DeckName
				);
			}

			Result = ButtonHandleEvent(
				UiContext,
				&SceneState->CollectionPrev,
				MouseEvent,
				MouseEventWorldPos
			);
			if(Result == ButtonHandleEvent_TakeAction)
			{
				CollectionCardsPrev(SceneState);
			}

			Result = ButtonHandleEvent(
				UiContext,
				&SceneState->CollectionNext,
				MouseEvent,
				MouseEventWorldPos
			);
			if(Result == ButtonHandleEvent_TakeAction)
			{
				CollectionCardsNext(SceneState);
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
	vector4 Black = Vector4(0.0f, 0.0f, 0.0f, 1.0f);

	if(!SceneState->DeckNameSet)
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
	else
	{
		PushText(
			&GameState->RenderGroup,
			&GameState->Assets,
			FontHandle_TestFont,
			SceneState->DeckName,
			SceneState->DeckNameBufferSize,
			20.0f,
			SceneState->DeckNamePos,
			White,
			&GameState->FrameArena
		);

		// NOTE: push collection cards
		collection_card* CollectionCards = SceneState->CollectionCards;
		uint32_t CollectionStartIndex = SceneState->CollectionStartIndex;
		card_definitions* Definitions = SceneState->Definitions;
		for(
			uint32_t Index = 0;
			Index < ARRAY_COUNT(SceneState->CollectionCards);
			Index++
		)
		{
			collection_card* CollectionCard = CollectionCards + Index;
			uint32_t CollectionIndex = CollectionStartIndex + Index;
			if(CollectionIndex < Definitions->NumCards)
			{
				card_definition* Definition = (
					Definitions->Array + CollectionIndex
				);
				PushButtonToRenderGroup(
					CollectionCard->Button.Rectangle,
					BitmapHandle_TestCard2,
					&GameState->RenderGroup,
					&GameState->Assets, 
					NULL,
					0,
					FontHandle_TestFont,
					White,
					NULL
				);

				if(IsHot(&SceneState->UiContext, CollectionCard->Button.Id))
				{
					PushInfoCard(
						&GameState->RenderGroup,
						&GameState->Assets,
						SceneState->InfoCardCenter,
						SceneState->InfoCardXBound,
						SceneState->InfoCardYBound,
						White,
						&GameState->FrameArena,
						Definition,
						-1
					);
				}
			}
		}

		// NOTE: push deck cards
		{
			char Buffer[256];
			deck_editor_cards* DeckCards = &SceneState->DeckCards;
			vector2 Dim = DeckCards->Dim;
			uint32_t ActiveCards = 0;
			for(int Index = 0; Index < MAX_CARDS_IN_DECK; Index++)
			{
				deck_editor_card* DeckCard = DeckCards->Cards + Index;
				if(IsActive(DeckCard))
				{
					rectangle Rectangle = MakeDeckCardRectangle(
						DeckCards, Dim, ActiveCards
					);

					ASSERT(DeckCard->Count > 0);
					if(DeckCard->Count == 1)
					{
						snprintf(
							Buffer,
							sizeof(Buffer),
							"%d",
							DeckCard->Definition->Id
						);
					}
					else if(DeckCard->Count > 1)
					{
						snprintf(
							Buffer,
							sizeof(Buffer),
							"%d x %d",
							DeckCard->Definition->Id,
							DeckCard->Count
						);
					}

					PushButtonToRenderGroup(
						Rectangle,
						BitmapHandle_TestCard2,
						&GameState->RenderGroup,
						&GameState->Assets, 
						Buffer,
						sizeof(Buffer),
						FontHandle_TestFont,
						Black,
						&GameState->FrameArena
					);

					if(IsHot(&SceneState->UiContext, DeckCard->UiId))
					{
						PushInfoCard(
							&GameState->RenderGroup,
							&GameState->Assets,
							SceneState->InfoCardCenter,
							SceneState->InfoCardXBound,
							SceneState->InfoCardYBound,
							White,
							&GameState->FrameArena,
							DeckCard->Definition,
							-1
						);	
					}

					ActiveCards++;
				}
			}
		}

		// NOTE: Push save button
		PushButtonToRenderGroup(
			SceneState->SaveButton.Rectangle,
			BitmapHandle_TestCard2,
			&GameState->RenderGroup,
			&GameState->Assets, 
			"Save Deck",
			sizeof("Save Deck"),
			FontHandle_TestFont,
			Black,
			&GameState->FrameArena
		);

		// NOTE: Push collection prev
		PushButtonToRenderGroup(
			SceneState->CollectionPrev.Rectangle,
			BitmapHandle_TestCard2,
			&GameState->RenderGroup,
			&GameState->Assets, 
			NULL,
			9,
			FontHandle_TestFont,
			White,
			NULL
		);
		// NOTE: PUsh collection next
		PushButtonToRenderGroup(
			SceneState->CollectionNext.Rectangle,
			BitmapHandle_TestCard2,
			&GameState->RenderGroup,
			&GameState->Assets, 
			NULL,
			9,
			FontHandle_TestFont,
			White,
			NULL
		);
	}

	PushCenteredAlert(&SceneState->Alert, GameState, BackBuffer);
}