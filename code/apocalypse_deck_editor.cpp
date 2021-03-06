#include "apocalypse_deck_editor.h"
#include "apocalypse_platform.h"
#include "apocalypse.h"
#include "apocalypse_info_card.h"
#include "apocalypse_card_definitions.h"
#include "apocalypse_deck_storage.h"
#include "apocalypse_render_group.h"
#include "apocalypse_sequence_alignment.h"

void SaveEditableDeck(
	game_state* GameState,
	alert* Alert,
	deck_editor_cards* DeckCards,
	char* DeckName
)
{
	loaded_deck Deck = {};
	uint8_t IdIndex = 0;
	uint8_t GeneralCount = 0;
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
			if(HasTag(&Card->Definition->GridTags, GridEffect_IsGeneral))
			{
				GeneralCount++;
			}
			Deck.Ids[IdIndex++] = Card->Definition->Id;
		}
	}
	ASSERT(IdIndex < 0xFF);
	if(GeneralCount == 0)
	{
		DisplayMessageFor(
			GameState, Alert, "No general in deck. Cannot save.", 1.0f
		);
	}
	else if(GeneralCount > 1)
	{
		DisplayMessageFor(
			GameState, Alert, "Too many generals in deck. Cannot save.", 1.0f
		);
	}
	else
	{
		Deck.Header.CardCount = IdIndex;
		char PathToDeck[256];
		FormatDeckPath(PathToDeck, sizeof(PathToDeck), DeckName);
		SaveDeck(PathToDeck, &Deck);
		DisplayMessageFor(GameState, Alert, "Saved Deck", 1.0f);
	}
}

void DeleteDeck(game_state* GameState, char* DeckName)
{
	char PathToDeck[256];
	FormatDeckPath(PathToDeck, sizeof(PathToDeck), DeckName);
	DeleteDeck(PathToDeck);
	StartDeckSelectorPrep(
		GameState,
		SceneType_DeckEditor,
		false,
		false,
		NULL,
		NULL
	);
}

uint32_t GetNumCardsInDeck(deck_editor_cards* DeckCards)
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
		SortingOn--
	)
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

void UpdateDeckScrollBar(
	deck_editor_cards* DeckCards, deck_editor_state* SceneState
)
{
	float AllDeckCardsHeight = GetAllDeckCardsHeight(DeckCards);
	scroll_bar* ScrollBar = &SceneState->DeckScrollBar;
	UpdateScrollBarPosDim(
		ScrollBar, DeckCards->YStart, AllDeckCardsHeight
	);
}

void RemoveCardFromDeck(
	deck_editor_cards* DeckCards,
	deck_editor_card* DeckCard,
	deck_editor_state* SceneState,
	ui_context* UiContext,
	vector2 MouseEventWorldPos
)
{
	DeckCard->Count--;

	// NOTE: need to recheck if buttons should be hot or active
	if(DeckCard->Count <= 0)
	{
		DeckCard->Definition = NULL;
		DeckCards->ActiveButtons--;

		SortDeckCards(DeckCards);
		UpdateDeckScrollBar(DeckCards, SceneState);
		vector2 Dim = DeckCards->Dim;
		for(
			uint32_t ButtonIndex = 0;
			ButtonIndex < ARRAY_COUNT(DeckCards->Cards);
			ButtonIndex++
		)
		{
			deck_editor_card* CheckCard = DeckCards->Cards + ButtonIndex;
			if(!IsActive(CheckCard))
			{
				break;
			}
			rectangle Rectangle = MakeDeckCardRectangle(
				DeckCards, Dim, ButtonIndex
			);

			ButtonHandleEvent(
				UiContext,
				CheckCard->UiId,
				Rectangle,
				MouseMove,
				MouseEventWorldPos
			);
		}
	}
}

void AddCardToDeck(
	game_state* GameState,
	deck_editor_state* SceneState,
	deck_editor_cards* DeckCards,
	card_definition* Definition
)
{
	if(GetNumCardsInDeck(DeckCards) >= MAX_CARDS_IN_DECK)
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
			if(!HasTag(&DeckCard->Definition->GridTags, GridEffect_IsGeneral))
			{
				DeckCard->Count++;
			}
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

void ScrollDeckCardPositions(
	deck_editor_state* SceneState, deck_editor_cards* DeckCards
)
{
	float AllDeckCardsHeight = GetAllDeckCardsHeight(DeckCards);
	DeckCards->YStart = GetElementsYStart(
		&SceneState->DeckScrollBar, AllDeckCardsHeight
	);
}

void LoadDeckForEditing(
	game_state* GameState, deck_editor_state* SceneState
)
{
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
}

void SortCollection(
	deck_editor_state* SceneState,
	char* SearchingFor,
	card_definitions* Definitions,
	card_definition** SortedDefinitions,
	memory_arena* FrameArena
)
{
	if(!SceneState->CollectionSorted)
	{
		SceneState->BrowseStartIndex = SceneState->CollectionStartIndex;
	}
	SceneState->CollectionStartIndex = 0;

	float* DefinitionSortKeys = PushArray(
		FrameArena, Definitions->NumCards, float
	);
	float* ScoreMemory = AllocScoreMemory(
		FrameArena, CARD_NAME_SIZE, CARD_NAME_SIZE
	);
	float MaxScore = 0.0f;
	for(uint32_t CardIndex = 0; CardIndex < Definitions->NumCards; CardIndex++)
	{
		card_definition* Definition = Definitions->Array + CardIndex;
		SortedDefinitions[CardIndex] = Definition;
		float AlignmentScore = SequenceAlignmentScore(
			SearchingFor,
			CARD_NAME_SIZE,
			Definition->Name,
			CARD_NAME_SIZE,
			ScoreMemory
		);
		if(AlignmentScore > MaxScore)
		{
			MaxScore = AlignmentScore;
		}
		DefinitionSortKeys[CardIndex] = AlignmentScore;
	}
	// NOTE: subtract off MaxScore if the first character we're searching for 
	// CONT: matches the first character in the name. we basically assume the 
	// CONT: user gets the first character correct 
	for(uint32_t CardIndex = 0; CardIndex < Definitions->NumCards; CardIndex++)
	{
		card_definition* Definition = Definitions->Array + CardIndex;
		SortedDefinitions[CardIndex] = Definition;
		if(Lower(SearchingFor[0]) == Lower(Definition->Name[0]))
		{
			DefinitionSortKeys[CardIndex] -= MaxScore;
		}
	}

	bool Sorted;
	do
	{
		Sorted = true;
		for(
			uint32_t CardIndex = 0;
			CardIndex < Definitions->NumCards - 1;
			CardIndex++
		)
		{
			float OldFirst = DefinitionSortKeys[CardIndex];
			float OldSecond = DefinitionSortKeys[CardIndex + 1];

			if(OldFirst > OldSecond)
			{
				DefinitionSortKeys[CardIndex] = OldSecond;
				DefinitionSortKeys[CardIndex + 1] = OldFirst;
				
				card_definition* OldFirstDefinition = (
					SortedDefinitions[CardIndex]
				);
				card_definition* OldSecondDefintion = (
					SortedDefinitions[CardIndex + 1]
				);

				SortedDefinitions[CardIndex] = OldSecondDefintion;
				SortedDefinitions[CardIndex + 1] = OldFirstDefinition;
				Sorted = false;
			}
		}
	} while(!Sorted);

	SceneState->CollectionSorted = true;
}

void RestoreBrowsing(deck_editor_state* SceneState)
{
	SceneState->CollectionStartIndex = SceneState->BrowseStartIndex;
	SceneState->CollectionSorted = false;
}

struct start_deck_editor_args
{
	char* DeckName;
	bool AlreadyExists;
};

void StartDeckEditorPrep(
	game_state* GameState, char* DeckName, bool AlreadyExists
)
{
	start_deck_editor_args* SceneArgs = PushStruct(
		&GameState->SceneArgsArena, start_deck_editor_args
	);

	SceneArgs->DeckName = PushArray(
		&GameState->SceneArgsArena, PLATFORM_MAX_PATH, char
	);
	strcpy_s(SceneArgs->DeckName, PLATFORM_MAX_PATH, DeckName);
	SceneArgs->AlreadyExists = AlreadyExists;

	GameState->SceneArgs = SceneArgs;
	GameState->Scene = SceneType_DeckEditor;
}

void StartDeckEditor(
	game_state* GameState, uint32_t WindowWidth, uint32_t WindowHeight
)
{
	start_deck_editor_args* SceneArgs = (start_deck_editor_args*) (
		GameState->SceneArgs
	);
	GameState->SceneState = PushStruct(
		&GameState->TransientArena, deck_editor_state
	);
	deck_editor_state* SceneState = (deck_editor_state*) GameState->SceneState;
	InitUiContext(&SceneState->UiContext);
	ui_context* UiContext = &SceneState->UiContext;

	vector2 ScreenDimInWorld = TransformVectorToBasis(
		&GameState->WorldToCamera,
		Vector2(WindowWidth, WindowHeight)
	);

	float DeckScrollBarWidth = 0.02f * ScreenDimInWorld.X;
	float DeckScrollBarMinX = WindowWidth - DeckScrollBarWidth;

	deck_editor_cards* DeckCards = &SceneState->DeckCards;
	*DeckCards = {};
	DeckCards->ActiveButtons = 0;
	DeckCards->Dim = {};
	DeckCards->Dim.X = 0.111f * ScreenDimInWorld.X;
	DeckCards->Dim.Y = 0.1875f * DeckCards->Dim.X;
	DeckCards->XPos = DeckScrollBarMinX - DeckCards->Dim.X;
	DeckCards->YStart = (float) WindowHeight;
	DeckCards->YMargin = 0.1f * DeckCards->Dim.Y;
	memset(
		DeckCards->Cards,
		0,
		ARRAY_COUNT(DeckCards->Cards) * sizeof(deck_editor_card)
	);

	rectangle DeckScrollBox = MakeRectangle(
		Vector2(DeckCards->XPos, 0.0f), 
		Vector2(DeckCards->Dim.X, (float) ScreenDimInWorld.Y)
	);
	scroll_bar* DeckScrollBar = &SceneState->DeckScrollBar;
	InitScrollBar(
		UiContext,
		DeckScrollBar,
		DeckScrollBarWidth,
		DeckScrollBox
	);

	SceneState->Definitions = DefineCards(&GameState->TransientArena);
	card_definitions* Definitions = SceneState->Definitions;
	SceneState->CollectionCardDim = {};
	SceneState->CollectionCardDim.X = 0.06f * ScreenDimInWorld.X;
	SceneState->CollectionCardDim.Y = 1.5f * SceneState->CollectionCardDim.X;
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
				rectangle* Rectangle = MakeTrackedRectangle(
					GameState,
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

	SceneState->SortedDefinitions = PushArray(
		&GameState->TransientArena,
		SceneState->Definitions->NumCards,
		card_definition* 
	);
	SceneState->SearchingFor = PushArray(
		&GameState->TransientArena, CARD_NAME_SIZE, char
	);
	memset(SceneState->SearchingFor, 0, CARD_NAME_SIZE * sizeof(char));
	InitTextInput(
		UiContext,
		&SceneState->SearchCollection,
		0.03f * ScreenDimInWorld.Y,
		SceneState->SearchingFor,
		CARD_NAME_SIZE
	);
	vector2 TempDim = Vector2(
		ScreenDimInWorld.X / 5.0f, SceneState->SearchCollection.FontHeight
	);
	SceneState->SearchCollectionRectangle = MakeRectangleCentered(
		Vector2(TempDim.X / 2.0f, ScreenDimInWorld.Y / 2.0f),
		TempDim
	);
	SceneState->CollectionSorted = false;

	vector2 BrowseCollectionButtonDim = Vector2(
		XOffset - 2 * XMargin, NumRows * (YMargin + Dim.Y)
	);
	InitButton(
		UiContext,
		&SceneState->CollectionPrev,
		MakeTrackedRectangle(
			GameState,
			Vector2(XMargin, YOffset),
			BrowseCollectionButtonDim
		)
	);

	InitButton(
		UiContext,
		&SceneState->CollectionNext,
		MakeTrackedRectangle(
			GameState,
			Vector2(NumCols * (Dim.X + XMargin) + XOffset, YOffset),
			BrowseCollectionButtonDim
		)
	);

	SceneState->DeckNameBufferSize = PLATFORM_MAX_PATH;
	SceneState->DeckName = PushArray(
		&GameState->TransientArena, SceneState->DeckNameBufferSize, char
	);
	memset(
		SceneState->DeckName, 0, SceneState->DeckNameBufferSize * sizeof(char)
	);
	strcpy_s(
		SceneState->DeckName,
		SceneState->DeckNameBufferSize,
		SceneArgs->DeckName
	);

	SceneState->Alert = MakeAlert();

	vector2 SaveButtonDim = Vector2(
		1.5f * SceneState->DeckCards.Dim.X, SceneState->DeckCards.Dim.Y
	);
	vector2 SaveButtonPos = Vector2(
		SceneState->DeckCards.XPos - SaveButtonDim.X, 0.0f
	);
	rectangle* SaveButtonRect = MakeTrackedRectangle(
		GameState, SaveButtonPos, SaveButtonDim
	);
	InitButton(UiContext, &SceneState->SaveButton, SaveButtonRect);

	vector2 DeleteButtonPos = SaveButtonPos;
	DeleteButtonPos.X -= SaveButtonDim.X + 5.0f; 
	rectangle* DeleteButtonRect = MakeTrackedRectangle(
		GameState, DeleteButtonPos, SaveButtonDim
	);
	InitButton(UiContext, &SceneState->DeleteButton, DeleteButtonRect);

	SceneState->CardCountPos = Vector2(
		GetCenter(*SaveButtonRect).X, 0.97f * ScreenDimInWorld.Y
	);

	SceneState->DeckNamePos = (
		GetTopLeft(*SaveButtonRect) + Vector2(0.0f, 0.033f * ScreenDimInWorld.Y)
	); 

	SceneState->InfoCardCenter = Vector2(
		ScreenDimInWorld.X / 2.0f, ScreenDimInWorld.Y / 2.0f
	);
	vector2 ScaledInfoCardDim = {};
	ScaledInfoCardDim.X = 0.14f * ScreenDimInWorld.X;
	ScaledInfoCardDim.Y = 1.5f * ScaledInfoCardDim.X;
	SceneState->InfoCardXBound = Vector2(ScaledInfoCardDim.X, 0.0f);
	SceneState->InfoCardYBound = Vector2(0.0f, ScaledInfoCardDim.Y);

	if(SceneArgs->AlreadyExists)
	{
		LoadDeckForEditing(GameState, SceneState);		
	}
}

void UpdateAndRenderDeckEditor(
	game_state* GameState,
	deck_editor_state* SceneState,
	uint32_t WindowWidth,
	uint32_t WindowHeight,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
)
{
	vector2 ScreenDimInWorld = TransformVectorToBasis(
		&GameState->WorldToCamera,
		Vector2(WindowWidth, WindowHeight)
	);
		
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

			button_handle_event_result Result;
			card_definitions* Definitions = SceneState->Definitions;
			card_definition** SortedDefinitions = (
				SceneState->SortedDefinitions
			);
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
				if(CollectionIndex < Definitions->NumCards)
				{
					Result = ButtonHandleEvent(
						UiContext,
						&CollectionCard->Button,
						MouseEvent->Type,
						MouseEventWorldPos
					);
					if(Result == ButtonHandleEvent_TakeAction)
					{
						card_definition* Definition;
						if(!SceneState->CollectionSorted)
						{
							Definition = (
								Definitions->Array + CollectionIndex
							);
						}
						else
						{
							Definition = *(
								SortedDefinitions + CollectionIndex
							);
						}
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
					MouseEvent->Type,
					MouseEventWorldPos
				);
				if(Result == ButtonHandleEvent_TakeAction)
				{
					RemoveCardFromDeck(
						DeckCards,
						DeckCard,
						SceneState,
						UiContext,
						MouseEventWorldPos
					);
					break;
				}
			}
			
			Result = ButtonHandleEvent(
				UiContext,
				&SceneState->SaveButton,
				MouseEvent->Type,
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
				&SceneState->DeleteButton,
				MouseEvent->Type,
				MouseEventWorldPos
			);
			if(Result == ButtonHandleEvent_TakeAction)
			{
				DeleteDeck(GameState, SceneState->DeckName);
			}

			Result = ButtonHandleEvent(
				UiContext,
				&SceneState->CollectionPrev,
				MouseEvent->Type,
				MouseEventWorldPos
			);
			if(Result == ButtonHandleEvent_TakeAction)
			{
				CollectionCardsPrev(SceneState);
			}

			Result = ButtonHandleEvent(
				UiContext,
				&SceneState->CollectionNext,
				MouseEvent->Type,
				MouseEventWorldPos
			);
			if(Result == ButtonHandleEvent_TakeAction)
			{
				CollectionCardsNext(SceneState);
			}

			float MinY = 0.0f;
			scroll_handle_mouse_code ScrollResult = ScrollHandleMouse(
				UiContext,
				&SceneState->DeckScrollBar,
				MouseEvent,
				MouseEventWorldPos,
				MinY, 
				SceneState->DeckScrollBar.Trough.Dim.Y
			);
			if(ScrollResult == ScrollHandleMouse_Moved)
			{
				ScrollDeckCardPositions(SceneState, DeckCards);
			}

			TextInputHandleMouse(
				UiContext,
				&SceneState->SearchCollection,
				SceneState->SearchCollectionRectangle,
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
				// NOTE: :common keyboard actions across all states
				switch(KeyboardEvent->Code)
				{
					case(0x1B): // NOTE: Escape V-code
					{
						GameState->Scene = SceneType_MainMenu; 
						break;
					}
				}

				text_input_kb_result KeyboardResult = TextInputHandleKeyboard(
					UiContext, &SceneState->SearchCollection, KeyboardEvent
				);
				if(KeyboardResult == TextInputKbResult_TextChanged)
				{
					if(strlen(SceneState->SearchingFor) > 0)
					{
						SortCollection(
							SceneState,
							SceneState->SearchingFor,
							SceneState->Definitions,
							SceneState->SortedDefinitions,
							&GameState->FrameArena
						);
					}
					else
					{
						RestoreBrowsing(SceneState);
					}
				}
			}

			UserEventIndex++;
		}
	}

	text_input_update_result UpdateResult = UpdateTextInput(
		UiContext, &SceneState->SearchCollection, DtForFrame
	);
	if(UpdateResult == TextInputUpdate_TextChanged)
	{
		if(strlen(SceneState->SearchingFor) > 0)
		{
			SortCollection(
				SceneState,
				SceneState->SearchingFor,
				SceneState->Definitions,
				SceneState->SortedDefinitions,
				&GameState->FrameArena
			);
		}
		else
		{
			RestoreBrowsing(SceneState);
		}
	}

	render_group* DefaultRenderGroup = &GameState->RenderGroup;
	assets* Assets = &GameState->Assets;
	PushClear(DefaultRenderGroup, Vector4(0.25f, 0.25f, 0.25f, 1.0f));
	
	vector4 White = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	vector4 Black = Vector4(0.0f, 0.0f, 0.0f, 1.0f);


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
	if(!SceneState->CollectionSorted)
	{
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
					*CollectionCard->Button.Rectangle,
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
	}
	else
	{
		uint32_t TotalDefined = SceneState->Definitions->NumCards;
		collection_card* CollectionCards = SceneState->CollectionCards;
		uint32_t CollectionStartIndex = SceneState->CollectionStartIndex;
		card_definition** Definitions = SceneState->SortedDefinitions;
		for(
			uint32_t Index = 0;
			Index < ARRAY_COUNT(SceneState->CollectionCards);
			Index++
		)
		{
			collection_card* CollectionCard = CollectionCards + Index;
			uint32_t CollectionIndex = CollectionStartIndex + Index;
			if(CollectionIndex < TotalDefined)
			{
				card_definition* Definition = *(
					Definitions + CollectionIndex
				);

				PushButtonToRenderGroup(
					*CollectionCard->Button.Rectangle,
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
		*SceneState->SaveButton.Rectangle,
		BitmapHandle_TestCard2,
		DefaultRenderGroup,
		Assets, 
		"Save Deck",
		sizeof("Save Deck"),
		FontHandle_TestFont,
		Black,
		&GameState->FrameArena
	);

	// NOTE: push delete button
	PushButtonToRenderGroup(
		*SceneState->DeleteButton.Rectangle,
		BitmapHandle_TestCard2,
		DefaultRenderGroup,
		Assets, 
		"Delete Deck",
		sizeof("Delete Deck"),
		FontHandle_TestFont,
		Black,
		&GameState->FrameArena
	);

	// NOTE: Push collection prev
	PushButtonToRenderGroup(
		*SceneState->CollectionPrev.Rectangle,
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
		*SceneState->CollectionNext.Rectangle,
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
	if(CanScroll(&SceneState->DeckScrollBar))
	{
		PushScrollBarToRenderGroup(
			SceneState->DeckScrollBar.Rect,
			BitmapHandle_TestCard2,
			DefaultRenderGroup,
			Assets
		);
	}

	PushTextInput(
		UiContext,
		&SceneState->SearchCollection,
		Vector4(1.0f, 1.0f, 1.0f, 1.0f),
		SceneState->SearchCollectionRectangle,
		BitmapHandle_TestCard2,
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Assets,
		DefaultRenderGroup,
		&GameState->FrameArena
	);

	// NOTE: push the number of cards that are currently in the deck vs. the
	// CONT: max number
	{
		uint32_t NumCards = GetNumCardsInDeck(&SceneState->DeckCards);
		char Buffer[32];
		snprintf(
			Buffer, sizeof(Buffer), "%d / %d", NumCards, MAX_CARDS_IN_DECK
		);
		PushTextCentered(
			DefaultRenderGroup,
			Assets,
			FontHandle_TestFont,
			Buffer,
			sizeof(Buffer),
			30.0f, 
			SceneState->CardCountPos,
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			&GameState->FrameArena 
		);
	}

	PushCenteredAlert(&SceneState->Alert, GameState, ScreenDimInWorld);
}