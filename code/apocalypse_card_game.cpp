#include "apocalypse.h"
#include "apocalypse_assets.h"
#include "apocalypse_render_group.h"
#include "apocalypse_main_menu.h"

#include "apocalypse_card_definitions.h"
#include "apocalypse_card_definitions.cpp"

#include "apocalypse_load_deck.h"
#include "apocalypse_load_deck.cpp"

#define MAX_RESOURCE_STRING_SIZE 40
void FormatResourceString(
	char* ResourceString, player_resources* PlayerResources
)
{
	snprintf(
		ResourceString,
		MAX_RESOURCE_STRING_SIZE,
		"R:%d\nG:%d\nBlu:%d\nW:%d\nBla:%d",
		PlayerResources->Resources[PlayerResource_Red],
		PlayerResources->Resources[PlayerResource_Green],
		PlayerResources->Resources[PlayerResource_Blue],
		PlayerResources->Resources[PlayerResource_White],
		PlayerResources->Resources[PlayerResource_Black]
	);
}

inline char* GetResourceInitial(player_resource_type ResourceType)
{
	switch(ResourceType)
	{
		case(PlayerResource_Red):
		{
			return "R";
		}
		case(PlayerResource_Green):
		{
			return "G";
		}
		case(PlayerResource_Blue):
		{
			return "Blu";
		}
		case(PlayerResource_White):
		{
			return "W";
		}
		case(PlayerResource_Black):
		{
			return "Bla";
		}
		default:
		{
			ASSERT(false);
			return NULL;
		}
	}
}

inline player_id GetOpponent(player_id Player)
{
	if(Player == Player_One)
	{
		return Player_Two;
	}
	else if(Player == Player_Two)
	{
		return Player_One;
	}
	else
	{
		ASSERT(false);
		return Player_Count;
	}
}

void DisplayMessageFor(game_state* GameState, char* Message, float Time)
{
	GameState->DisplayMessageUntil = GameState->Time + Time;
	strcpy_s(
		GameState->MessageBuffer,
		ARRAY_COUNT(GameState->MessageBuffer),
		Message
	);	
}

void CannotActivateCardMessage(game_state* GameState)
{
	DisplayMessageFor(
		GameState, "Cannot activate card. Too few resources", 1.0f
	);
}

bool AddCardToSet(card_set* CardSet, card* Card)
{
	bool Result = false;
	for(int Index = 0; Index < ARRAY_COUNT(CardSet->Cards); Index++)
	{
		if(CardSet->Cards[Index] == Card)
		{
			Result = false;
			break;
		}
		else if(CardSet->Cards[Index] == NULL)
		{
			CardSet->Cards[Index] = Card;
			CardSet->CardCount++;
			Result = true;
			break;
		}
	}
	Card->SetType = CardSet->Type;
	return Result;
}

void AlignCardSet(card_set* CardSet)
{
	// NOTE: Basically, this is the amount of space not taken up by the 
	// CONT: cards divided by the number of spaces between and around the 
	// CONT: cards
	float Width = CardSet->CardWidth;
	float SpaceSize = (
		(CardSet->ScreenWidth - (CardSet->CardCount * Width)) / 
		(CardSet->CardCount + 1)
	);
	float DistanceBetweenCardPos = SpaceSize + Width;
	float CurrentXPos = SpaceSize;
	for(int Index = 0; Index < ARRAY_COUNT(CardSet->Cards); Index++)
	{
		card* Card = CardSet->Cards[Index];
		if(Card != NULL && Card->Active)
		{
			Card->Rectangle.Min.X = CurrentXPos;
			Card->Rectangle.Min.Y = CardSet->YPos;
			CurrentXPos += DistanceBetweenCardPos;
		}
	}
}

void AddCardAndAlign(card_set* CardSet, card* Card)
{
	if(AddCardToSet(CardSet, Card))
	{
		AlignCardSet(CardSet);		
	}
}

bool RemoveCardFromSet(card_set* CardSet, card* Card)
{
	for(int Index = 0; Index < ARRAY_COUNT(CardSet->Cards); Index++)
	{
		if(CardSet->Cards[Index] == Card)
		{
			CardSet->Cards[Index] = NULL;
			CardSet->CardCount--;
			return true;
		}
	}
	return false;
}

void RemoveCardAndAlign(card_set* CardSet, card* Card)
{
	if(RemoveCardFromSet(CardSet, Card))
	{
		AlignCardSet(CardSet);
	}
}

card* GetInactiveCard(game_state* GameState)
{
	card* Card = NULL;
	for(
		int SearchIndex = 0;
		SearchIndex < GameState->MaxCards;
		SearchIndex++
	)
	{
		Card = &GameState->Cards[SearchIndex];
		if(!Card->Active)
		{
			break;
		}
	}
	ASSERT(Card != NULL);
	ASSERT(!Card->Active);

	Card->Rectangle.Dim.X = GameState->CardWidth;
	Card->Rectangle.Dim.Y = GameState->CardHeight;
	Card->TimeLeft = 10.0f;
	Card->Active = true;
	vector4* Color = &Card->Color;
	Color->A = 1.0f;
	Color->R = 1.0f;
	Color->G = 1.0f;
	Color->B = 1.0f;
	return Card;
}

void InitCardWithDeckCard(deck* Deck, card* Card, player_id Owner)
{
	deck_card* CardToDraw = Deck->InDeck;
	ASSERT(CardToDraw != NULL);	
	for(
		int PlayerIndex = 0;
		PlayerIndex < Player_Count;
		PlayerIndex++
	)
	{
		card_definition* Definition = CardToDraw->Definition;
		Card->Definition = Definition;
	
		Card->PlayDelta[PlayerIndex] = Definition->PlayDelta[PlayerIndex];
		Card->TapDelta[PlayerIndex] = Definition->TapDelta[PlayerIndex];
		Card->TapsAvailable = Definition->TapsAvailable;
		Card->Attack = Definition->Attack;
		Card->Health = Definition->Health;
	}
	Card->Owner = Owner;
	InDeckToOutDeck(Deck, CardToDraw);
}

void DrawCard(game_state* GameState, player_id Owner)
{
	// NOTE: can't exceed maximum hand size
	card_set* CardSet = &GameState->Hands[GameState->CurrentTurn];
	if(CardSet->CardCount >= MAX_CARDS_PER_SET)
	{
		DisplayMessageFor(GameState, "Can't draw card. Hand full", 1.0f);
		return;
	}
	deck* Deck = &GameState->Decks[GameState->CurrentTurn];
	
	card* Card = GetInactiveCard(GameState);
	InitCardWithDeckCard(Deck, Card, Owner);
	AddCardAndAlign(CardSet, Card);
}

void DrawFullHand(game_state* GameState, player_id Player)
{
	deck* Deck = &GameState->Decks[Player];
	for(
		int CardIndex = 0;
		CardIndex < MAX_CARDS_PER_SET;
		CardIndex++
	)
	{
		card* Card = GetInactiveCard(GameState);
		InitCardWithDeckCard(Deck, Card, Player);
		AddCardToSet(&GameState->Hands[Player], Card);
		Card++;
	}
	AlignCardSet(&GameState->Hands[Player]);
}

void AppendResourceStringToInfoCard(
	card* Card,
	player_resources* Deltas,
	string_appender* StringAppender,
	char* SelfOrOpp,
	char* DeltaType
)
{
	bool NonZeroDelta = false;
	for(
		int DeltaIndex = 0;
		DeltaIndex < PlayerResource_Count;
		DeltaIndex++
	)
	{
		uint32_t Delta = Deltas->Resources[DeltaIndex];
		if(Delta != 0)
		{
			if(!NonZeroDelta)
			{
				AppendToString(
					StringAppender, "%s: %s\n", SelfOrOpp, DeltaType
				);
				NonZeroDelta = true;
			}
			AppendToString(
				StringAppender,
				"%s: %d\n",
				GetResourceInitial((player_resource_type) DeltaIndex),
				Delta
			);
		}
	}
}

void InitDeckCard(
	deck_card* DeckCard, card_definition* Definitions, uint32_t CardId
)
{
	DeckCard->Definition = Definitions + CardId;
}

bool CheckAndActivate(
	player_resources* PlayerResources, player_resources* Deltas, card* Card
)
{
	// NOTE: only activate card if you have the resources for it
	player_resources* ChangeTarget = &PlayerResources[Card->Owner];
	player_resources* Delta = &Deltas[Card->Owner];
	bool Result = CanChangeResources(ChangeTarget, Delta);
	if(Result)
	{
		ChangeResources(ChangeTarget, Delta);
		player_id Opp = GetOpponent(Card->Owner);
		ChangeTarget = &PlayerResources[Opp];
		Delta = &Deltas[Opp];
		ChangeResources(ChangeTarget, Delta);
	}
	return Result;
}

void SelectCard(game_state* GameState, card* Card)
{
	Card->Color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
	GameState->SelectedCard = Card;
}

void DeselectCard(game_state* GameState)
{
	if(GameState->SelectedCard == NULL)
	{
		return;
	}
	card* Card = GameState->SelectedCard;
	Card->Color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	GameState->SelectedCard = NULL;
}

void AttackCard(game_state* GameState, card* AttackingCard, card* AttackedCard)
{
	// NOTE: this currently assumes cards must attack from the tableau
	AttackingCard->Health -= AttackedCard->Attack;
	AttackedCard->Health -= AttackingCard->Attack;

	if(AttackingCard->Health <= 0)
	{
		RemoveCardAndAlign(
			&GameState->Tableaus[AttackingCard->Owner], AttackingCard
		);
		AttackingCard->Active = false;
	}
	if(AttackedCard->Health <= 0)
	{
		RemoveCardAndAlign(
			&GameState->Tableaus[AttackedCard->Owner], AttackedCard
		);
		AttackedCard->Active = false;
	}
}

void StartCardGame(game_state* GameState, game_offscreen_buffer* BackBuffer)
{
	loaded_deck P1Deck;
	loaded_deck P2Deck;
	for(
		int DeckCardIndex = 0;
		DeckCardIndex < MAX_CARDS_IN_DECK;
		DeckCardIndex++
	)
	{
		P1Deck.Ids[DeckCardIndex] = 0;
		P2Deck.Ids[DeckCardIndex] = 0;
	}
	P1Deck.Header.CardCount = MAX_CARDS_IN_DECK;
	P2Deck.Header.CardCount = MAX_CARDS_IN_DECK;
	// TODO: Remove Save decks. should only happen in deck editor
	SaveDeck("../decks/P1Deck.deck", &P1Deck);
	SaveDeck("../decks/P2Deck.deck", &P2Deck);
	// TODO: load decks based on interaction at start of new card game
	P1Deck = LoadDeck("../decks/P1Deck.deck");
	P2Deck = LoadDeck("../decks/P2Deck.deck");

	GameState->MaxCards = Player_Count * CardSet_Count * MAX_CARDS_PER_SET;
	GameState->Cards = PushArray(
		&GameState->Arena, GameState->MaxCards, card
	);
	{
		GameState->Definitions = DefineCards(&GameState->TransientArena);
		GameState->Decks = PushArray(&GameState->Arena, Player_Count, deck);
		for(
			int PlayerIndex = Player_One;
			PlayerIndex < Player_Count;
			PlayerIndex++
		)
		{
			loaded_deck* LoadedDeck;
			if(PlayerIndex == Player_One)
			{
				LoadedDeck = &P1Deck;
			}
			else
			{
				LoadedDeck = &P2Deck;
			}
			deck* Deck = &GameState->Decks[PlayerIndex];
			*Deck = {};

			deck_card* DeckCard = &Deck->Cards[0];
			*DeckCard = {};
			InitDeckCard(
				DeckCard, GameState->Definitions, LoadedDeck->Ids[0]
			);
			DeckCard->Next = NULL;
			DeckCard->Previous = NULL;
			Deck->OutOfDeck = DeckCard;
			Deck->OutOfDeckLength++;
			for(
				int CardIndex = 1;
				CardIndex < LoadedDeck->Header.CardCount;
				CardIndex++
			)
			{
				deck_card* OldHead = Deck->OutOfDeck;
				DeckCard = &Deck->Cards[CardIndex];
				*DeckCard = {};
				InitDeckCard(
					DeckCard,
					GameState->Definitions,
					LoadedDeck->Ids[CardIndex]
				);

				DeckCard->Next = OldHead;
				DeckCard->Previous = NULL;
				OldHead->Previous = DeckCard;
				Deck->OutOfDeck = DeckCard;
				Deck->OutOfDeckLength++;
			}
		}

		// SECTION START: Shuffle deck
		for(
			int PlayerIndex = Player_One;
			PlayerIndex < Player_Count;
			PlayerIndex++
		)
		{
			deck* Deck = &GameState->Decks[PlayerIndex];

			while(Deck->OutOfDeckLength > 0)
			{
				int DeckCardIndex = rand() % Deck->OutOfDeckLength;
				deck_card* CardToShuffle = GetDeckCard(
					Deck->OutOfDeck, Deck->OutOfDeckLength, DeckCardIndex
				);
				OutDeckToInDeck(Deck, CardToShuffle);
			}
		}
		// SECTION STOP: Shuffle deck
	}

	GameState->PlayerResources = PushArray(
		&GameState->TransientArena, Player_Count, player_resources
	);
	GameState->Hands = PushArray(
		&GameState->TransientArena, Player_Count, card_set
	);
	GameState->Tableaus = PushArray(
		&GameState->TransientArena, Player_Count, card_set
	);

	float CardWidth = 60.0f;
	float CardHeight = 90.0f;
	GameState->CardWidth = CardWidth;
	GameState->CardHeight = CardHeight;

	float HandTableauMargin = 5.0f;
	// NOTE: transform assumes screen and camera are 1:1
	vector2 ScreenDimInWorld = TransformVectorToBasis(
		&GameState->WorldToCamera,
		Vector2(BackBuffer->Width, BackBuffer->Height)
	);
	float ScreenWidthInWorld = ScreenDimInWorld.X;
	
	card_set* CardSet = &GameState->Hands[Player_One];
	CardSet->Type = CardSet_Hand;
	CardSet->CardCount = 0;
	CardSet->ScreenWidth = ScreenWidthInWorld;
	CardSet->YPos = 0.0f;
	CardSet->CardWidth = CardWidth;

	CardSet = &GameState->Hands[Player_Two];
	CardSet->Type = CardSet_Hand;
	CardSet->CardCount = 0;
	CardSet->ScreenWidth = ScreenWidthInWorld;
	CardSet->YPos = ScreenDimInWorld.Y - CardHeight;
	CardSet->CardWidth = CardWidth;

	CardSet = &GameState->Tableaus[Player_One];
	CardSet->Type = CardSet_Tableau;
	CardSet->CardCount = 0;
	CardSet->ScreenWidth = ScreenWidthInWorld;
	CardSet->YPos = CardHeight + HandTableauMargin;
	CardSet->CardWidth = CardWidth;

	CardSet = &GameState->Tableaus[Player_Two];
	CardSet->Type = CardSet_Tableau;
	CardSet->CardCount = 0;
	CardSet->ScreenWidth = ScreenWidthInWorld;
	CardSet->YPos = (
		ScreenDimInWorld.Y - (2 * CardHeight) - HandTableauMargin 
	);
	CardSet->CardWidth = CardWidth;
	
	GameState->InfoCardCenter = Vector2(
		BackBuffer->Width / 2.0f, BackBuffer->Height / 2.0f
	);

	vector2 ScaledInfoCardDim = 0.33f * Vector2(600.0f, 900.0f);
	GameState->InfoCardXBound = Vector2(ScaledInfoCardDim.X, 0.0f);
	GameState->InfoCardYBound = Vector2(0.0f, ScaledInfoCardDim.Y);

	DrawFullHand(GameState, Player_One);
	DrawFullHand(GameState, Player_Two);

	GameState->TurnTimer = 20.0f;

	for(int PlayerIndex = 0; PlayerIndex < Player_Count; PlayerIndex++)
	{
		player_resources* PlayerResources = (
			&GameState->PlayerResources[PlayerIndex]
		);
		// TODO: initialize to 0?
		SetResource(PlayerResources, PlayerResource_Red, rand() % 10 + 1);
		SetResource(PlayerResources, PlayerResource_Green, rand() % 10 + 1);
		SetResource(PlayerResources, PlayerResource_Blue, rand() % 10 + 1);
		SetResource(PlayerResources, PlayerResource_White, rand() % 10 + 1);
		SetResource(PlayerResources, PlayerResource_Black, rand() % 10 + 1);
	}

	// TODO: remove me!
	// PlaySound(
	// 	&GameState->PlayingSoundList,
	// 	WavHandle_TestMusic,
	// 	&GameState->TransientArena
	// );

	// TODO: remove me!
	{
		vector2 ScreenCenter = Vector2(
			BackBuffer->Width / 2.0f, BackBuffer->Height / 2.0f
		);
		GameState->TestParticleSystem = MakeParticleSystem(
			&GameState->TransientArena,
			BitmapHandle_TestBitmap,
			1.0f,
			Vector2(35.0f, 35.0f),
			Vector2(45.0f, 45.0f),
			ScreenCenter - Vector2(5.0f, 5.0f),
			ScreenCenter + Vector2(5.0f, 5.0f),
			PI32,
			2 * PI32,
			1.0f,
			5.0f,
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, -0.01f),
			Vector4(0.0f, 0.0f, 0.0f, -0.05f),
			0.25f,
			256
		);
	}

	GameState->Scene = SceneType_CardGame;
}

void UpdateAndRenderCardGame(
	game_state* GameState,
	game_offscreen_buffer* BackBuffer,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
)
{
	GameState->Time += DtForFrame;
	bool EndTurn = false;

	// SECTION START: User input
	// TODO: move to game memory
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
			if(MouseEvent->Type == PrimaryUp)
			{
				PlaySound(
					&GameState->PlayingSoundList,
					WavHandle_Bloop00,
					&GameState->TransientArena
				);
				card* Card = &GameState->Cards[0];
				for(
					int CardIndex = 0;
					CardIndex < GameState->MaxCards;
					CardIndex++
				)
				{
					if(
						Card->Active &&
						PointInRectangle(MouseEventWorldPos, Card->Rectangle)
					)
					{
						// NOTE: player clicked their own card on their turn 
						if(Card->Owner == GameState->CurrentTurn)
						{
							if(Card->SetType == CardSet_Hand)
							{
								bool WasActivated = CheckAndActivate(
									GameState->PlayerResources,
									Card->PlayDelta,
									Card
								);
								if(WasActivated)
								{
									RemoveCardAndAlign(
										&GameState->Hands[Card->Owner], Card
									);
									AddCardAndAlign(
										&GameState->Tableaus[Card->Owner], Card
									);
								}
								else
								{
									CannotActivateCardMessage(GameState);
								}
							}
							else if(Card->SetType == CardSet_Tableau)
							{
								if(GameState->SelectedCard == NULL)
								{
									if(Card->TimesTapped < Card->TapsAvailable)
									{
										player_resources* ChangeTarget = (
											&GameState->PlayerResources[Card->Owner]
										);
										player_resources* Delta = (
											&Card->TapDelta[Card->Owner]
										);
										bool CanActivate = CanChangeResources(
											ChangeTarget, Delta
										);										
										if(CanActivate)
										{
											SelectCard(GameState, Card);
										}
										else
										{
											CannotActivateCardMessage(
												GameState
											);
										}
									}
								}
								else
								{
									if(GameState->SelectedCard == Card)
									{
										DeselectCard(GameState);
									}
									else
									{
										DeselectCard(GameState);
										SelectCard(GameState, Card);
									}
								}
							}
							else
							{
								ASSERT(false);
							}
							break;
						}
						// NOTE: player clicked on opponent's card on their turn
						else
						{
							if(
								GameState->SelectedCard && 
								Card->SetType == CardSet_Tableau
							)
							{
								card* SelectedCard = GameState->SelectedCard;
								player_id Owner = SelectedCard->Owner;
								CheckAndActivate(
									&GameState->PlayerResources[Owner], 
									&SelectedCard->TapDelta[Owner],
									GameState->SelectedCard
								);
								SelectedCard->TimesTapped++;
								DeselectCard(GameState);

								AttackCard(GameState, SelectedCard, Card);
							}
						}
					}
					Card++;
				}
			}
			else if(MouseEvent->Type == MouseMove)
			{
				card* Card = &GameState->Cards[0];
				for(
					int CardIndex = 0;
					CardIndex < GameState->MaxCards;
					CardIndex++
				)
				{
					if(Card->Active)
					{
						if(
							PointInRectangle(
								MouseEventWorldPos, Card->Rectangle
							)
						)
						{
							Card->HoveredOver = true;							
						}
						else
						{
							Card->HoveredOver = false;
						}
					}
					Card++;
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
					case(0x0D): // NOTE: Return/Enter V-code
					{
						if(!KeyboardEvent->IsDown && KeyboardEvent->WasDown)
						{
							EndTurn = true;
						}
						break;
					}
					case(0x1B): // NOTE: Escape V-code
					{
						StartMainMenu(GameState);
						break;
					}
					case(0x44): // NOTE: D V-code
					{
						// TODO: consider pulling this predicate out to an 
						// CONT: inline function called "KeyUp" or something
						if(!KeyboardEvent->IsDown && KeyboardEvent->WasDown)
						{
							// TODO: use console commands or something for 
							// CONT: debug mode
							GameState->OverlayDebugInfo = (
								!GameState->OverlayDebugInfo
							);
						}
						break;
					}					
					case(0x00):
					{

					}
				}
			}

			UserEventIndex++;
		}
	}
	// SECTION STOP: User input

	// SECTION START: Updating game state
	// NOTE: not sure if we should finish all updates and then push to the 
	// CONT: render group
	// TODO: remove scaling test code
	// float ScaleValue = cosf((2 * PI32 / 5.0f) * GameState->Time);
	// GameState->WorldToCamera = MakeBasis(
	// 	Vector2(BackBuffer->Width / 2.0f, BackBuffer->Height / 2.0f),
	// 	ScaleValue * Vector2(1.0f, 0.0f),
	// 	ScaleValue * Vector2(0.0f, 1.0f)
	// );
	// SECTION START: Turn timer update
	if(!EndTurn)
	{
		GameState->TurnTimer -= DtForFrame;
		// NOTE: switch turns
		if(GameState->TurnTimer <= 0)
		{
			EndTurn = true;
		}
	}
	if(EndTurn)
	{
		GameState->TurnTimer = 20.0f;
		GameState->CurrentTurn = (
			(GameState->CurrentTurn	== Player_Two) ? Player_One : Player_Two
		);
		for(int CardIndex = 0; CardIndex < GameState->MaxCards; CardIndex++)
		{
			card* Card = &GameState->Cards[CardIndex];
			if(Card->Active)
			{
				Card->TimesTapped = 0;
			}
		}
		DrawCard(GameState, GameState->CurrentTurn);
	}
	// SECTION STOP: Turn timer update
	// SECTION START: Card update
	{
		card* Card = &GameState->Cards[0];
		for(
			int CardIndex = 0;
			CardIndex < GameState->MaxCards;
			CardIndex++
		)
		{
			if(Card->Active)
			{
				Card->TimeLeft -= DtForFrame;
			}
			Card++;
		}
	}
	// SECTION STOP: Card update

	// SECTION START: Push render entries
	PushClear(&GameState->RenderGroup, Vector4(0.25f, 0.25f, 0.25f, 1.0f));
	// SECTION START: Push turn timer
	{
		int32_t TurnTimerCeil = Int32Ceil(GameState->TurnTimer);
		uint32_t MaxTurnTimerCharacters = 10;
		char* TurnTimerString = PushArray(
			&GameState->FrameArena, MaxTurnTimerCharacters, char
		);
		char* PlayerIndicator = NULL;
		if(GameState->CurrentTurn == Player_One)
		{
			PlayerIndicator = "P1";
		}
		else if(GameState->CurrentTurn == Player_Two)
		{
			PlayerIndicator = "P2";
		}
		ASSERT(PlayerIndicator != NULL);
		snprintf(
			TurnTimerString,
			MaxTurnTimerCharacters,
			"%s: %d",
			PlayerIndicator,
			TurnTimerCeil
		);
		PushText(
			&GameState->RenderGroup,
			&GameState->Assets,
			FontHandle_TestFont,
			TurnTimerString,
			MaxTurnTimerCharacters,
			50.0f,
			Vector2(
				BackBuffer->Width - 150.0f, 
				(BackBuffer->Height / 2.0f)
			),
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			&GameState->FrameArena
		);
	}
	// SECTION STOP: Push turn timer
	// SECTION START: Push cards
	{
		card* Card = &GameState->Cards[0];
		for(
			int CardIndex = 0;
			CardIndex < GameState->MaxCards;
			CardIndex++
		)
		{
			if(Card->Active)
			{
				PushSizedBitmap(
					&GameState->RenderGroup,
					&GameState->Assets,
					BitmapHandle_TestCard2,
					GetCenter(Card->Rectangle),
					Vector2(Card->Rectangle.Dim.X, 0.0f),
					Vector2(0.0f, Card->Rectangle.Dim.Y),
					Card->Color
				);
				if(Card->HoveredOver)
				{
					PushSizedBitmap(
						&GameState->RenderGroup,
						&GameState->Assets,
						BitmapHandle_TestCard2,
						GameState->InfoCardCenter,
						GameState->InfoCardXBound,
						GameState->InfoCardYBound,
						Card->Color
					);

					#define ATTACK_HEALTH_MAX_LENGTH 8
					uint32_t MaxCharacters = (
						2 * ATTACK_HEALTH_MAX_LENGTH + 
						4 * MAX_RESOURCE_STRING_SIZE
					);
					char* ResourceString = PushArray(
						&GameState->FrameArena,
						MaxCharacters,
						char
					);
					string_appender StringAppender = MakeStringAppender(
						ResourceString, MaxCharacters 
					);

					AppendToString(
						&StringAppender, "Attack: %d\n", Card->Attack
					);
					AppendToString(
						&StringAppender, "Health: %d\n", Card->Health
					);

					AppendResourceStringToInfoCard(
						Card,
						&Card->PlayDelta[Card->Owner],
						&StringAppender,
						"Self",
						"Play"
					);
					AppendResourceStringToInfoCard(
						Card,
						&Card->TapDelta[Card->Owner],
						&StringAppender,
						"Self",
						"Tap"
					);

					player_id Opp = GetOpponent(Card->Owner);
					AppendResourceStringToInfoCard(
						Card,
						&Card->PlayDelta[Opp],
						&StringAppender,
						"Opp",
						"Play"
					);
					AppendResourceStringToInfoCard(
						Card,
						&Card->TapDelta[Opp],
						&StringAppender,
						"Opp",
						"Tap"
					);

					if(Card->SetType == CardSet_Tableau)
					{
						AppendToString(
							&StringAppender,
							"TapsLeft: %d\n",
							Card->TapsAvailable - Card->TimesTapped
						);
					}

					vector2 TopLeft = (
						GameState->InfoCardCenter - 
						0.5f * GameState->InfoCardXBound + 
						0.5f * GameState->InfoCardYBound
					);
					PushTextTopLeft(
						&GameState->RenderGroup,
						&GameState->Assets,
						FontHandle_TestFont,
						ResourceString,
						MaxCharacters,
						20.0f,
						TopLeft,
						Vector4(0.0f, 0.0f, 0.0f, 1.0f),
						&GameState->FrameArena
					);
				}
			}
			Card++;
		}
	}
	// SECTION STOP: Push cards
	// SECTION START: Push resources
	{
		char* ResourceString = PushArray(
			&GameState->FrameArena, MAX_RESOURCE_STRING_SIZE, char
		);
		FormatResourceString(
			ResourceString, &GameState->PlayerResources[Player_One]
		);
		float Padding = 15.0f;
		PushText(
			&GameState->RenderGroup,
			&GameState->Assets,
			FontHandle_TestFont,
			ResourceString,
			MAX_RESOURCE_STRING_SIZE,
			15.0f,
			Vector2(
				BackBuffer->Width - 50.0f, 
				BackBuffer->Height / 2.0f - Padding
			),
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			&GameState->FrameArena
		);

		FormatResourceString(
			ResourceString, &GameState->PlayerResources[Player_Two]
		);
		PushText(
			&GameState->RenderGroup,
			&GameState->Assets,
			FontHandle_TestFont,
			ResourceString,
			MAX_RESOURCE_STRING_SIZE,
			15.0f,
			Vector2(
				BackBuffer->Width - 50.0f, 
				(BackBuffer->Height / 2.0f) + 80.0f + Padding
			),
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			&GameState->FrameArena
		);
	}
	// SECTION STOP: Push resources

	// SECTION START: Display message
	if(GameState->Time < GameState->DisplayMessageUntil)
	{
		PushTextCentered(
			&GameState->RenderGroup,
			&GameState->Assets,
			FontHandle_TestFont,
			GameState->MessageBuffer,
			ARRAY_COUNT(GameState->MessageBuffer),
			50.0f,
			Vector2(
				BackBuffer->Width / 2.0f, 
				(BackBuffer->Height / 2.0f)
			),
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			&GameState->FrameArena
		);
	}
	// SECTION STOP: Display message

#if 0 // NOTE: tests for bitmaps
	PushSizedBitmap(
		&GameState->RenderGroup,
		&GameState->Assets,
		BitmapHandle_TestBackground,
		Vector2((BackBuffer->Width / 2.0f), (BackBuffer->Height / 2.0f)),
		Vector2(BackBuffer->Width, 0),
		Vector2(0, BackBuffer->Height),
		Vector4(1.0f, 1.0f, 1.0f, 1.0f)		
	);

	float RotationalPeriod = 2.0f;
	float Radians = (2 * PI32 * GameState->Time) / RotationalPeriod;
	float CosVal = cosf(Radians);
	float SinVal = sinf(Radians);
	vector2 Origin = Vector2(0.0f, BackBuffer->Height / 2.0f);
	vector2 Center = Vector2(
		(BackBuffer->Width / 2.0f), (BackBuffer->Height / 2.0f)
	);
	vector2 XAxis = Vector2(CosVal, SinVal);
	vector2 YAxis = Perpendicular(XAxis);

	PushCenteredBitmap(
		&GameState->RenderGroup,
		&GameState->Assets,
		BitmapHandle_TestBitmap,
		Center,
		XAxis,
		YAxis,
		Vector4(1.0f, 1.0f, 1.0f, 1.0f)
	);

	PushText(
		&GameState->RenderGroup,
		&GameState->Assets,
		FontHandle_TestFont,
		"A hello world\nA test is here\nAnother line\nA\nA\nA\nA\nA",
		256,
		50.0f,
		Center + Vector2(0.0f, (BackBuffer->Height / 4.0f)),
		Vector4(1.0f, 1.0f, 1.0f, 1.0f),
		&GameState->FrameArena
	);

	basis RectBasis = MakeBasis(Vector2(0, 0), Vector2(1, 0), Vector2(0, 1));
	PushRect(
		&GameState->RenderGroup,
		&RectBasis,
		MakeRectangle(Vector2(0, 0), Vector2(25, 25)),
		Vector4(1.0f, 1.0f, 1.0f, 1.0f)
	);
#endif

#if 0 // NOTE: tests for particle systems
	UpdateParticleSystem(&GameState->TestParticleSystem);
	PushParticles(
		&GameState->RenderGroup,
		&GameState->Assets,
		&GameState->TestParticleSystem
	);
#endif

	// SECTION STOP: Updating game state
}