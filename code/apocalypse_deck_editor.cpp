#include "apocalypse_deck_editor.h"
#include "apocalypse_platform.h"
#include "apocalypse.h"
#include "apocalypse_info_card.h"
#include "apocalypse_card_definitions.h"
#include "apocalypse_deck_storage.h"
#include "apocalypse_render_group.h"

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
		uint32_t CardIndex = 0; CardIndex < MAX_CARDS_IN_DECK; CardIndex++
	)
	{
		deck_editor_card* Card = Cards + CardIndex;
		if(!IsActive(Card))
		{
			break;
		}
		ASSERT(Card->Count > 0);
		for(uint32_t Added = 0; Added < Card->Count; Added++)
		{
			Deck.Ids[IdIndex++] = Card->Definition->Id;
		}
	}
	ASSERT(IdIndex < 0xFF);
	Deck.Header.CardCount = IdIndex;

	char PathToDeck[256];
	FormatDeckPath(PathToDeck, sizeof(PathToDeck), DeckName);
	SaveDeck(PathToDeck, &Deck);
	DisplayMessageFor(GameState, Alert, "Saved Deck", 1.0f);
}

uint32_t GetCardsInDeck(deck_editor_cards* DeckCards)
{
	uint32_t CardsInDeck = 0;
	for(int Index = 0; Index < MAX_CARDS_IN_DECK; Index++)
	{
		deck_editor_card* Card = DeckCards->Cards + Index;
		if(!IsActive(Card))
		{
			break;
		}
		ASSERT(Card->Count > 0);
		CardsInDeck += Card->Count;
	}
	return CardsInDeck;
}

void SwapDeckCards(deck_editor_card* CardPtr, deck_editor_card* NextCardPtr)
{
	deck_editor_card Card = *CardPtr;
	deck_editor_card NextCard = *NextCardPtr;
	*CardPtr = NextCard;
	*NextCardPtr = Card;
}

void SortDeckCards(deck_editor_cards* DeckCards)
{
	// NOTE: alphabetical sort
	// NOTE: using bubble sort b/c the data can't get that big
	// NOTE: b/c we're doing a lot of swapping anyway, this code also fills in 
	// CONT: the gaps in the deck_editor array so all adjacent stuff is active
	for(
		int32_t SortingOn = CARD_NAME_SIZE - 1; 
		SortingOn >= 0;
		SortingOn--)
	{
		bool Sorted;
		do
		{
			Sorted = true;
			for(
				uint32_t CardIndex = 0;
				CardIndex < MAX_CARDS_IN_DECK - 1;
				CardIndex++
			)
			{
				deck_editor_card* CardPtr = DeckCards->Cards + CardIndex;
				card_definition* Definition = CardPtr->Definition;
				deck_editor_card* NextCardPtr = CardPtr + 1;
				card_definition* NextDefinition = NextCardPtr->Definition;

				if(Definition == NULL && NextDefinition != NULL)
				{
					SwapDeckCards(CardPtr, NextCardPtr);
					Sorted = false;
				}
				else if(Definition != NULL && NextDefinition != NULL)
				{
					char CardNameChar = *(Definition->Name + SortingOn);
					char NextNameChar = *(NextDefinition->Name + SortingOn);

					char CardNameLower = Lower(CardNameChar);
					char NextNameLower = Lower(NextNameChar);
					if(CardNameLower > NextNameLower)
					{
						SwapDeckCards(CardPtr, NextCardPtr);
						Sorted = false;
					}
				}
			}
		} while(!Sorted);
	}
}

rectangle MakeNextRectangle(
	float XPos, float YStart, float YMargin, vector2 Dim, uint32_t Index
)
{
	// NOTE: Makes the next vertical rectangle in a vertical stack of 
	// CONT: rectangles
	return MakeRectangle(
		Vector2(XPos, YStart - (Dim.Y + YMargin) * (Index + 1)), Dim
	);
}

rectangle MakeDeckCardRectangle(
	deck_editor_cards* DeckCards, vector2 Dim, uint32_t ButtonIndex
)
{
	return MakeNextRectangle(
		DeckCards->XPos, DeckCards->YStart, DeckCards->YMargin, Dim, ButtonIndex
	);
}

float GetAllDeckCardsHeight(deck_editor_cards* DeckCards)
{
	rectangle FirstDeckCard = MakeDeckCardRectangle(
		DeckCards, DeckCards->Dim, 0
	);
	rectangle LastDeckCard = MakeDeckCardRectangle(
		DeckCards, DeckCards->Dim, DeckCards->ActiveButtons - 1
	);
	return GetTop(FirstDeckCard) - GetBottom(LastDeckCard);
}

bool IsScrollBarInteractable(deck_editor_state* SceneState)
{
	return SceneState->DeckScrollBarRect.Dim.Y < SceneState->MaxDeckScrollBarY;
}

void UpdateDeckScrollBar(
	deck_editor_cards* DeckCards, deck_editor_state* SceneState
)
{
	float AllDeckCardsHeight = GetAllDeckCardsHeight(DeckCards);
	float MaxDeckScrollBarY = SceneState->MaxDeckScrollBarY;
	UpdateScrollBarDim(
		&SceneState->DeckScrollBarRect,
		MaxDeckScrollBarY / AllDeckCardsHeight, 
		MaxDeckScrollBarY
	);

	float FractionSeenStartFromTop = (
		(DeckCards->YStart - SceneState->DeckScrollBarTop) / 
		AllDeckCardsHeight
	);
	SetTop(
		&SceneState->DeckScrollBarRect, 
		(
			SceneState->DeckScrollBarTop - 
			FractionSeenStartFromTop * MaxDeckScrollBarY
		)
	);
}

void RemoveCardFromDeck(
	deck_editor_cards* DeckCards,
	deck_editor_card* DeckCard,
	deck_editor_state* SceneState
)
{
	DeckCard->Count--;

	if(DeckCard->Count <= 0)
	{
		DeckCard->Definition = NULL;
		DeckCards->ActiveButtons--;
	}
	SortDeckCards(DeckCards);
	UpdateDeckScrollBar(DeckCards, SceneState);
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
		if(!IsActive(DeckCard))
		{
			break;
		}
		
		if(DeckCard->Definition == Definition)
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
	for(
		int Index = DeckCards->ActiveButtons;
		Index < MAX_CARDS_IN_DECK;
		Index++
	)
	{
		DeckCard = DeckCards->Cards + Index;
		if(!IsActive(DeckCard))
		{
			DeckCard->UiId = GetId(&SceneState->UiContext);
			DeckCard->Definition = Definition;
			DeckCard->Count = 1;
			DeckCards->ActiveButtons++;
			break;
		}
	}

	SortDeckCards(DeckCards);
	UpdateDeckScrollBar(DeckCards, SceneState);
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

void StartDeckEditor(game_state* GameState, game_offscreen_buffer* BackBuffer)
{
	ResetMemArena(&GameState->TransientArena);
	GameState->SceneState = PushStruct(
		&GameState->TransientArena, deck_editor_state
	);
	ResetAssets(&GameState->Assets);
	deck_editor_state* SceneState = (deck_editor_state*) GameState->SceneState;
	InitUiContext(&SceneState->UiContext);
	ui_context* UiContext = &SceneState->UiContext;

	InitScrollBar(UiContext, &SceneState->DeckScrollBar);
	vector2 DeckScrollBarDim = Vector2(30.0f, 0.0f);
	vector2 DeckScrollBarMin = Vector2(
		BackBuffer->Width - DeckScrollBarDim.X, 0.0f
	);
	SceneState->DeckScrollBarRect = MakeRectangle(
		DeckScrollBarMin, DeckScrollBarDim
	);
	SceneState->DeckScrollBarTop = (float) BackBuffer->Height;
	SceneState->MaxDeckScrollBarY = (float) BackBuffer->Height;
	
	deck_editor_cards* DeckCards = &SceneState->DeckCards;
	*DeckCards = {};
	DeckCards->ActiveButtons = 0;
	DeckCards->Dim = Vector2(160.0f, 30.0f);
	DeckCards->XPos = DeckScrollBarMin.X - DeckCards->Dim.X;
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
					UiContext,
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
		UiContext,
		&SceneState->CollectionPrev,
		ScrollButtonRectangle
	);

	ScrollButtonRectangle = MakeRectangle(
		Vector2(NumCols * (Dim.X + XMargin) + XOffset, YOffset),
		BrowseCollectionButtonDim
	);
	InitButton(
		UiContext,
		&SceneState->CollectionNext,
		ScrollButtonRectangle
	);

	SceneState->DeckNameBufferSize = 32;
	SceneState->DeckName = PushArray(
		&GameState->TransientArena, SceneState->DeckNameBufferSize, char
	);
	SceneState->DeckNameSet = false;

	text_input* TextInput = &SceneState->DeckNameInput;
	{
		*TextInput = {};
		ClearAllFlags(TextInput);
		TextInput->UiId = GetId(UiContext);
		TextInput->CursorPos = 0;
		TextInput->FontHeight = 20.0f;
		TextInput->BufferSize = SceneState->DeckNameBufferSize;
		TextInput->Buffer = SceneState->DeckName;
		TextInput->RepeatDelay = 1.0f;
		TextInput->RepeatPeriod = 0.05f;
		TextInput->CursorColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		TextInput->CursorAlphaState = CursorAlphaState_Decreasing;
	}
	SceneState->DeckNameInputRectangle = MakeRectangleCentered(
		Vector2(BackBuffer->Width / 2.0f, BackBuffer->Height / 2.0f),
		Vector2(BackBuffer->Width / 5.0f, TextInput->FontHeight)
	);
	SetActive(UiContext, TextInput->UiId);

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

	vector2 SaveButtonDim = Vector2(
		1.5f * SceneState->DeckCards.Dim.X, SceneState->DeckCards.Dim.Y
	);
	rectangle SaveButtonRectangle = MakeRectangle(
		Vector2(SceneState->DeckCards.XPos - SaveButtonDim.X, 0.0f),
		SaveButtonDim
	);
	InitButton(
		UiContext,
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
			if(!SceneState->DeckNameSet)
			{
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
						SceneState->DeckNameSet = true;
						// TODO: add the deck's cards to the editor
						loaded_deck LoadedDeck = {};
						char DeckPath[PLATFORM_MAX_PATH];
						FormatDeckPath(
							DeckPath,
							ARRAY_COUNT(DeckPath),
							SceneState->DeckName
						);
						LoadedDeck = LoadDeck(DeckPath);
						for(
							uint32_t CardIndex = 0;
							CardIndex < LoadedDeck.Header.CardCount;
							CardIndex++
						)
						{
							card_definition* Definition = (
								SceneState->Definitions->Array + 
								LoadedDeck.Ids[CardIndex]
							);
							AddCardToDeck(
								GameState,
								SceneState,
								&SceneState->DeckCards,
								Definition
							);
						}
						break;
					}
					else
					{
						CurrentDeckName = GetNextString(&FlatArrayReader); 
					}
				}
			}
			else
			{
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
				for(
					uint32_t ButtonIndex = 0;
					ButtonIndex < ARRAY_COUNT(DeckCards->Cards);
					ButtonIndex++
				)
				{
					deck_editor_card* DeckCard = DeckCards->Cards + ButtonIndex;
					if(!IsActive(DeckCard))
					{
						break;
					}
					rectangle Rectangle = MakeDeckCardRectangle(
						DeckCards, Dim, ButtonIndex
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
						RemoveCardFromDeck(DeckCards, DeckCard, SceneState);
						break;
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

				if(IsScrollBarInteractable(SceneState))
				{
					scroll_bar_handle_mouse_code ScrollBarResult = (
						ScrollBarHandleMouse(
							UiContext,
							&SceneState->DeckScrollBar,
							&SceneState->DeckScrollBarRect,
							MouseEvent,
							MouseEventWorldPos,
							0.0f,
							SceneState->DeckScrollBarTop
						)
					);
					if(ScrollBarResult == ScrollBarHandleMouse_Moved)
					{
						float AllDeckCardsHeight = (
							GetAllDeckCardsHeight(DeckCards)
						);
						float FractionSeenStartFromTop = (
							(
								GetTop(SceneState->DeckScrollBarRect) - 
								SceneState->DeckScrollBarTop
							) /
							SceneState->MaxDeckScrollBarY
						); 
						DeckCards->YStart = (
							SceneState->DeckScrollBarTop - 
							(FractionSeenStartFromTop * AllDeckCardsHeight)
						);
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
				text_input_kb_result KeyboardResult = TextInputHandleKeyboard(
					UiContext, &SceneState->DeckNameInput, KeyboardEvent
				);
				if(KeyboardResult == TextInputKbResult_Submit)
				{
					SceneState->DeckNameSet = true;
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

	if(!SceneState->DeckNameSet)
	{
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
	}
	else
	{
		PushText(
			DefaultRenderGroup,
			Assets,
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
					DefaultRenderGroup,
					Assets, 
					Definition->Name,
					sizeof(Definition->Name),
					FontHandle_TestFont,
					12.0f,
					Black,
					&GameState->FrameArena
				);

				if(IsHot(UiContext, CollectionCard->Button.Id))
				{
					PushInfoCard(
						DefaultRenderGroup,
						Assets,
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
			char Buffer[128];
			deck_editor_cards* DeckCards = &SceneState->DeckCards;
			vector2 Dim = DeckCards->Dim;
			for(int Index = 0; Index < MAX_CARDS_IN_DECK; Index++)
			{
				deck_editor_card* DeckCard = DeckCards->Cards + Index;
				if(!IsActive(DeckCard))
				{
					break;
				}
				
				rectangle Rectangle = MakeDeckCardRectangle(
					DeckCards, Dim, Index
				);

				ASSERT(DeckCard->Count > 0);
				if(DeckCard->Count == 1)
				{
					snprintf(
						Buffer,
						sizeof(Buffer),
						"%s",
						DeckCard->Definition->Name
					);
				}
				else if(DeckCard->Count > 1)
				{
					snprintf(
						Buffer,
						sizeof(Buffer),
						"%s x %d",
						DeckCard->Definition->Name,
						DeckCard->Count
					);
				}

				PushButtonToRenderGroup(
					Rectangle,
					BitmapHandle_TestCard2,
					DefaultRenderGroup,
					Assets, 
					Buffer,
					sizeof(Buffer),
					FontHandle_TestFont,
					Black,
					&GameState->FrameArena
				);

				if(IsHot(&SceneState->UiContext, DeckCard->UiId))
				{
					PushInfoCard(
						DefaultRenderGroup,
						Assets,
						SceneState->InfoCardCenter,
						SceneState->InfoCardXBound,
						SceneState->InfoCardYBound,
						White,
						&GameState->FrameArena,
						DeckCard->Definition,
						-1
					);	
				}
			}
		}

		// NOTE: Push save button
		PushButtonToRenderGroup(
			SceneState->SaveButton.Rectangle,
			BitmapHandle_TestCard2,
			DefaultRenderGroup,
			Assets, 
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
			DefaultRenderGroup,
			Assets, 
			NULL,
			9,
			FontHandle_TestFont,
			White,
			NULL
		);
		// NOTE: Push collection next
		PushButtonToRenderGroup(
			SceneState->CollectionNext.Rectangle,
			BitmapHandle_TestCard2,
			DefaultRenderGroup,
			Assets, 
			NULL,
			9,
			FontHandle_TestFont,
			White,
			NULL
		);

		// NOTE: Push deck editor scroll bar
		if(IsScrollBarInteractable(SceneState))
		{
			PushScrollBarToRenderGroup(
				SceneState->DeckScrollBarRect,
				BitmapHandle_TestCard2,
				DefaultRenderGroup,
				Assets
			);
		}
	}

	PushCenteredAlert(&SceneState->Alert, GameState, BackBuffer);
}