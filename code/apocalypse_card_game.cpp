#include "apocalypse_card_game.h"
#include "apocalypse.h"
#include "apocalypse_assets.h"
#include "apocalypse_render_group.h"
#include "apocalypse_main_menu.h"
#include "apocalypse_button.h"
#include "apocalypse_deck_storage.h"
#include "apocalypse_card_definitions.h"
#include "apocalypse_info_card.h"
#include "apocalypse_alert.h"

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

void CannotActivateCardMessage(
	game_state* GameState, alert* Alert
)
{
	DisplayMessageFor(
		GameState, Alert, "Cannot activate card. Too few resources", 1.0f
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

card* GetInactiveCard(card_game_state* SceneState)
{
	card* Card = NULL;
	for(
		int SearchIndex = 0;
		SearchIndex < SceneState->MaxCards;
		SearchIndex++
	)
	{
		Card = &SceneState->Cards[SearchIndex];
		if(!Card->Active)
		{
			break;
		}
	}
	ASSERT(Card != NULL);
	ASSERT(!Card->Active);

	Card->Rectangle.Dim.X = SceneState->CardWidth;
	Card->Rectangle.Dim.Y = SceneState->CardHeight;
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
		Card->EffectTags = Definition->Tags;
	}
	Card->Owner = Owner;
	InDeckToOutDeck(Deck, CardToDraw);
}

void DrawCard(
	game_state* GameState, card_game_state* SceneState, player_id Owner
)
{
	// NOTE: can't exceed maximum hand size
	card_set* CardSet = &SceneState->Hands[SceneState->CurrentTurn];
	if(CardSet->CardCount >= MAX_CARDS_PER_SET)
	{
		DisplayMessageFor(
			GameState, &SceneState->Alert, "Can't draw card. Hand full", 1.0f
		);
		return;
	}
	deck* Deck = &SceneState->Decks[SceneState->CurrentTurn];
	
	card* Card = GetInactiveCard(SceneState);
	InitCardWithDeckCard(Deck, Card, Owner);
	AddCardAndAlign(CardSet, Card);
}

void DrawFullHand(card_game_state* SceneState, player_id Player)
{
	deck* Deck = &SceneState->Decks[Player];
	for(
		int CardIndex = 0;
		CardIndex < MAX_CARDS_PER_SET;
		CardIndex++
	)
	{
		card* Card = GetInactiveCard(SceneState);
		InitCardWithDeckCard(Deck, Card, Player);
		AddCardToSet(&SceneState->Hands[Player], Card);
		Card++;
	}
	AlignCardSet(&SceneState->Hands[Player]);
}

void InitDeckCard(
	deck_card* DeckCard, card_definitions* Definitions, uint32_t CardId
)
{
	// TODO: Might want a way to protect against card id out of range
	ASSERT(CardId < Definitions->NumCards);
	DeckCard->Definition = Definitions->Array + CardId;
}

bool CheckAndTapLand(
	game_state* GameState, card_game_state* SceneState, card* Card
)
{
	bool Tapped = false;
	int32_t SelfResourceDelta = SumResources(
		&Card->TapDelta[Player_One]
	);
	int32_t OppResourceDelta = SumResources(
		&Card->TapDelta[Player_Two]
	);
	float TimeChange = 5.0f * (SelfResourceDelta - OppResourceDelta);
	if(TimeChange <= GameState->Time)
	{
		SceneState->TurnTimer -= TimeChange;
		Tapped = true;
	}
	return Tapped;
}

bool CheckAndTap(
	game_state* GameState, card_game_state* SceneState, card* Card
)
{
	player_resources* PlayerResources = (
		&SceneState->PlayerResources[Card->Owner]
	);
	player_resources* Deltas = &Card->TapDelta[Card->Owner];
	
	// NOTE: only activate card if you have the resources for it
	player_resources* ChangeTarget = &PlayerResources[Card->Owner];
	player_resources* Delta = &Deltas[Card->Owner];
	bool Tapped = CanChangeResources(ChangeTarget, Delta);
	if(Tapped)
	{
		if(HasTag(&Card->EffectTags, CardEffect_Land))
		{
			Tapped = CheckAndTapLand(GameState, SceneState, Card);
		}

		if(Tapped)
		{
			ChangeResources(ChangeTarget, Delta);
			player_id Opp = GetOpponent(Card->Owner);
			ChangeTarget = &PlayerResources[Opp];
			Delta = &Deltas[Opp];
			ChangeResources(ChangeTarget, Delta);
		}
	}
	return Tapped;
}

bool CheckAndPlay(
	game_state* GameState, card_game_state* SceneState, card* Card
)
{
	// NOTE: only activate card if you have the resources for it
	player_resources* PlayerResources = SceneState->PlayerResources;
	player_resources* Deltas = Card->PlayDelta;

	player_resources* ChangeTarget = &PlayerResources[Card->Owner];
	player_resources* Delta = &Deltas[Card->Owner];
	bool Played = CanChangeResources(ChangeTarget, Delta);
	if(Played)
	{
		// TODO: may need a faster card lookup
		char* CardName = Card->Definition->Name;
		
		// NOTE: card effects on activation can be added here
		// TODO: may need a faster card lookup
		
		if(Played)
		{
			ChangeResources(ChangeTarget, Delta);
			player_id Opp = GetOpponent(Card->Owner);
			ChangeTarget = &PlayerResources[Opp];
			Delta = &Deltas[Opp];
			ChangeResources(ChangeTarget, Delta);
		}
	}
	return Played;
}

void SelectCard(card_game_state* SceneState, card* Card)
{
	Card->Color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
	SceneState->SelectedCard = Card;
}

void DeselectCard(card_game_state* SceneState)
{
	if(SceneState->SelectedCard == NULL)
	{
		return;
	}
	card* Card = SceneState->SelectedCard;
	Card->Color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	SceneState->SelectedCard = NULL;
}

void AttackCard(
	card_game_state* SceneState, card* AttackingCard, card* AttackedCard
)
{
	// NOTE: this currently assumes cards must attack from the tableau
	AttackingCard->Health -= AttackedCard->Attack;
	AttackedCard->Health -= AttackingCard->Attack;

	if(AttackingCard->Health <= 0)
	{
		RemoveCardAndAlign(
			&SceneState->Tableaus[AttackingCard->Owner], AttackingCard
		);
		AttackingCard->Active = false;
	}
	if(AttackedCard->Health <= 0)
	{
		RemoveCardAndAlign(
			&SceneState->Tableaus[AttackedCard->Owner], AttackedCard
		);
		AttackedCard->Active = false;
	}
}

void SetTurnTimer(card_game_state* SceneState, float Value)
{
	SceneState->TurnTimer = Value;
	uint16_t IntTurnTimer = (uint16_t) SceneState->TurnTimer;
	SceneState->LastWholeSecond = IntTurnTimer;
}

void StartCardGamePrep(
	game_state* GameState, char* P1DeckName, char* P2DeckName
)
{
	start_card_game_args* SceneArgs = PushStruct(
		&GameState->SceneArgsArena, start_card_game_args
	);

	char Buffer[PLATFORM_MAX_PATH];
	FormatDeckPath(Buffer, sizeof(Buffer), P1DeckName);
	SceneArgs->P1Deck = LoadDeck(Buffer);
	FormatDeckPath(Buffer, sizeof(Buffer), P2DeckName);
	SceneArgs->P2Deck = LoadDeck(Buffer);

	GameState->SceneArgs = SceneArgs;
	GameState->Scene = SceneType_CardGame; 
}

void StartCardGame(game_state* GameState, game_offscreen_buffer* BackBuffer)
{
	start_card_game_args* SceneArgs = (start_card_game_args*) (
		GameState->SceneArgs
	);
	loaded_deck P1Deck = SceneArgs->P1Deck;
	loaded_deck P2Deck = SceneArgs->P2Deck;

	ResetMemArena(&GameState->TransientArena);
	GameState->SceneState = PushStruct(
		&GameState->TransientArena, card_game_state
	);
	ResetAssets(&GameState->Assets);

	card_game_state* SceneState = (card_game_state*) GameState->SceneState;
	*SceneState = {};

	SceneState->MaxCards = Player_Count * CardSet_Count * MAX_CARDS_PER_SET;
	SceneState->Cards = PushArray(
		&GameState->TransientArena, SceneState->MaxCards, card
	);
	memset(SceneState->Cards, 0, SceneState->MaxCards * sizeof(card));

	{
		SceneState->Definitions = DefineCards(&GameState->TransientArena);
		SceneState->Decks = PushArray(
			&GameState->TransientArena, Player_Count, deck
		);
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
			deck* Deck = &SceneState->Decks[PlayerIndex];
			*Deck = {};

			deck_card* DeckCard = &Deck->Cards[0];
			*DeckCard = {};
			InitDeckCard(
				DeckCard, SceneState->Definitions, LoadedDeck->Ids[0]
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
					SceneState->Definitions,
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
			deck* Deck = &SceneState->Decks[PlayerIndex];

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

	SceneState->PlayerResources = PushArray(
		&GameState->TransientArena, Player_Count, player_resources
	);
	SceneState->Hands = PushArray(
		&GameState->TransientArena, Player_Count, card_set
	);
	memset(SceneState->Hands, 0, Player_Count * sizeof(card_set));
	SceneState->Tableaus = PushArray(
		&GameState->TransientArena, Player_Count, card_set
	);

	float CardWidth = 60.0f;
	float CardHeight = 90.0f;
	SceneState->CardWidth = CardWidth;
	SceneState->CardHeight = CardHeight;

	float HandTableauMargin = 5.0f;
	// NOTE: transform assumes screen and camera are 1:1
	vector2 ScreenDimInWorld = TransformVectorToBasis(
		&GameState->WorldToCamera,
		Vector2(BackBuffer->Width, BackBuffer->Height)
	);
	float ScreenWidthInWorld = ScreenDimInWorld.X;
	
	card_set* CardSet = &SceneState->Hands[Player_One];
	CardSet->Type = CardSet_Hand;
	CardSet->CardCount = 0;
	CardSet->ScreenWidth = ScreenWidthInWorld;
	CardSet->YPos = 0.0f;
	CardSet->CardWidth = CardWidth;

	CardSet = &SceneState->Hands[Player_Two];
	CardSet->Type = CardSet_Hand;
	CardSet->CardCount = 0;
	CardSet->ScreenWidth = ScreenWidthInWorld;
	CardSet->YPos = ScreenDimInWorld.Y - CardHeight;
	CardSet->CardWidth = CardWidth;

	CardSet = &SceneState->Tableaus[Player_One];
	CardSet->Type = CardSet_Tableau;
	CardSet->CardCount = 0;
	CardSet->ScreenWidth = ScreenWidthInWorld;
	CardSet->YPos = CardHeight + HandTableauMargin;
	CardSet->CardWidth = CardWidth;

	CardSet = &SceneState->Tableaus[Player_Two];
	CardSet->Type = CardSet_Tableau;
	CardSet->CardCount = 0;
	CardSet->ScreenWidth = ScreenWidthInWorld;
	CardSet->YPos = (
		ScreenDimInWorld.Y - (2 * CardHeight) - HandTableauMargin 
	);
	CardSet->CardWidth = CardWidth;
	
	SceneState->InfoCardCenter = Vector2(
		BackBuffer->Width / 2.0f, BackBuffer->Height / 2.0f
	);

	vector2 ScaledInfoCardDim = 0.33f * Vector2(600.0f, 900.0f);
	SceneState->InfoCardXBound = Vector2(ScaledInfoCardDim.X, 0.0f);
	SceneState->InfoCardYBound = Vector2(0.0f, ScaledInfoCardDim.Y);

	DrawFullHand(SceneState, Player_One);
	DrawFullHand(SceneState, Player_Two);

	SetTurnTimer(SceneState, 20.0f);

	for(int PlayerIndex = 0; PlayerIndex < Player_Count; PlayerIndex++)
	{
		player_resources* PlayerResources = (
			&SceneState->PlayerResources[PlayerIndex]
		);
		// TODO: initialize to 0?
		SetResource(PlayerResources, PlayerResource_Red, rand() % 10 + 1);
		SetResource(PlayerResources, PlayerResource_Green, rand() % 10 + 1);
		SetResource(PlayerResources, PlayerResource_Blue, rand() % 10 + 1);
		SetResource(PlayerResources, PlayerResource_White, rand() % 10 + 1);
		SetResource(PlayerResources, PlayerResource_Black, rand() % 10 + 1);
	}

	SceneState->Alert = MakeAlert();

	// TODO: remove me!
	// PlaySound(
	// 	&GameState->PlayingSoundList,
	// 	WavHandle_TestMusic,
	// 	&GameState->TransientArena
	// );
}

void UpdateAndRenderCardGame(
	game_state* GameState,
	card_game_state* SceneState,
	game_offscreen_buffer* BackBuffer,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
)
{
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
				card* Card = &SceneState->Cards[0];
				for(
					int CardIndex = 0;
					CardIndex < SceneState->MaxCards;
					CardIndex++
				)
				{
					if(
						Card->Active &&
						PointInRectangle(MouseEventWorldPos, Card->Rectangle)
					)
					{
						card* SelectedCard = SceneState->SelectedCard;

						// NOTE: player clicked their own card on their turn 
						if(Card->Owner == SceneState->CurrentTurn)
						{
							if(Card->SetType == CardSet_Hand)
							{
								bool WasPlayed = CheckAndPlay(
									GameState, SceneState, Card
								);
								if(WasPlayed)
								{
									RemoveCardAndAlign(
										&SceneState->Hands[Card->Owner], Card
									);
									AddCardAndAlign(
										&SceneState->Tableaus[Card->Owner], Card
									);
								}
								else
								{
									CannotActivateCardMessage(
										GameState, &SceneState->Alert
									);
								}
							}
							else if(Card->SetType == CardSet_Tableau)
							{
								if(SelectedCard == NULL)
								{
									if(Card->TimesTapped < Card->TapsAvailable)
									{
										player_resources* ChangeTarget = (
											&SceneState->PlayerResources[Card->Owner]
										);
										player_resources* Delta = (
											&Card->TapDelta[Card->Owner]
										);
										bool CanActivate = CanChangeResources(
											ChangeTarget, Delta
										);										
										if(CanActivate)
										{
											SelectCard(SceneState, Card);
										}
										else
										{
											CannotActivateCardMessage(
												GameState, &SceneState->Alert
											);
										}
									}
								}
								else
								{
									if(SelectedCard == Card)
									{
										// NOTE: We may want to have a more 
										// CONT: specific check than this
										if(SelectedCard->Attack == 0)
										{
											player_id Owner = SelectedCard->Owner;
											DeselectCard(SceneState);
											CheckAndTap(
												GameState,
												SceneState,
												SelectedCard
											);
											SelectedCard->TimesTapped++;
										}
									}
									else
									{
										DeselectCard(SceneState);
										SelectCard(SceneState, Card);
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
								SelectedCard != NULL && 
								Card->SetType == CardSet_Tableau
							)
							{
								player_id Owner = SelectedCard->Owner;
								CheckAndTap(
									GameState,
									SceneState,
									SelectedCard
								);
								SelectedCard->TimesTapped++;
								DeselectCard(SceneState);

								AttackCard(SceneState, SelectedCard, Card);
							}
						}
					}
					Card++;
				}
			}
			else if(MouseEvent->Type == SecondaryUp)
			{
				if(SceneState->SelectedCard != NULL)
				{
					DeselectCard(SceneState);
				}
			}
			else if(MouseEvent->Type == MouseMove)
			{
				card* Card = &SceneState->Cards[0];
				for(
					int CardIndex = 0;
					CardIndex < SceneState->MaxCards;
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
						GameState->Scene = SceneType_MainMenu; 
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
	// SECTION START: Turn timer update
	bool WholeSecondPassed = false;
	if(!EndTurn)
	{
		SceneState->TurnTimer -= DtForFrame;
		// NOTE: switch turns
		if(SceneState->TurnTimer <= 0)
		{
			EndTurn = true;
		}
		else
		{
			uint16_t IntTurnTimer = (uint16_t) SceneState->TurnTimer;
			WholeSecondPassed = (
				(SceneState->LastWholeSecond - IntTurnTimer) >= 1
			);
			if(WholeSecondPassed)
			{
				SceneState->LastWholeSecond = IntTurnTimer;
			}
		}
	}
	if(EndTurn)
	{
		SetTurnTimer(SceneState, 20.0f);
		SceneState->CurrentTurn = (
			(SceneState->CurrentTurn == Player_Two) ? Player_One : Player_Two
		);
		for(int CardIndex = 0; CardIndex < SceneState->MaxCards; CardIndex++)
		{
			card* Card = &SceneState->Cards[CardIndex];
			if(Card->Active)
			{
				Card->TimesTapped = 0;
			}
		}
		DrawCard(GameState, SceneState, SceneState->CurrentTurn);
	}
	// SECTION STOP: Turn timer update
	// SECTION START: Card update
	{
		card* Card = &SceneState->Cards[0];
		for(
			int CardIndex = 0;
			CardIndex < SceneState->MaxCards;
			CardIndex++
		)
		{
			if(Card->Active)
			{
				Card->TimeLeft -= DtForFrame;

				card_effect_tags* EffectTags = &Card->EffectTags;
				if(HasTag(EffectTags, CardEffect_SelfWeaken))
				{
					if(
						Card->Owner == SceneState->CurrentTurn && 
						Card->SetType == CardSet_Tableau
					)
					{
						if(WholeSecondPassed)
						{
							Card->Attack -= 1;
						}
					}
				}
				else if(HasTag(EffectTags, CardEffect_OppStrengthen))
				{
					if(
						Card->Owner != SceneState->CurrentTurn && 
						Card->SetType == CardSet_Tableau
					)
					{
						if(WholeSecondPassed)
						{
							Card->Attack += 1;
						}
					}
				}
				else if(HasTag(EffectTags, CardEffect_SelfLifeLoss))
				{
					if(
						Card->Owner == SceneState->CurrentTurn && 
						Card->SetType == CardSet_Tableau
					)
					{
						if(WholeSecondPassed)
						{
							Card->Health -= 1;
						}
						if(Card->Health <= 0)
						{
							RemoveCardAndAlign(
								&SceneState->Tableaus[Card->Owner], Card
							);
							Card->Active = false;
						}
					}
				}
				else if(HasTag(EffectTags, CardEffect_OppLifeGain))
				{
					if(
						Card->Owner != SceneState->CurrentTurn && 
						Card->SetType == CardSet_Tableau
					)
					{
						if(WholeSecondPassed)
						{
							Card->Health += 1;
						}						
					}
				}
			}
			Card++;
		}
	}
	// SECTION STOP: Card update
	// SECTION STOP: Updating game state

	// SECTION START: Push render entries
	PushClear(&GameState->RenderGroup, Vector4(0.25f, 0.25f, 0.25f, 1.0f));

	// SECTION START: Push turn timer
	{
		int32_t TurnTimerCeil = Int32Ceil(SceneState->TurnTimer);
		uint32_t MaxTurnTimerCharacters = 10;
		char* TurnTimerString = PushArray(
			&GameState->FrameArena, MaxTurnTimerCharacters, char
		);
		char* PlayerIndicator = NULL;
		if(SceneState->CurrentTurn == Player_One)
		{
			PlayerIndicator = "P1";
		}
		else if(SceneState->CurrentTurn == Player_Two)
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
		card* Card = &SceneState->Cards[0];
		for(
			int CardIndex = 0;
			CardIndex < SceneState->MaxCards;
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
					PushInfoCard(
						&GameState->RenderGroup,
						&GameState->Assets,
						SceneState->InfoCardCenter,
						SceneState->InfoCardXBound,
						SceneState->InfoCardYBound,
						Card->Color,
						&GameState->FrameArena,
						Card->Definition->Name,
						Card->Attack,
						Card->Health,
						Card->PlayDelta,
						Card->TapDelta,
						Card->Definition->Name,
						Card->TapsAvailable - Card->TimesTapped
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
			ResourceString, &SceneState->PlayerResources[Player_One]
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
			ResourceString, &SceneState->PlayerResources[Player_Two]
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

	PushCenteredAlert(&SceneState->Alert, GameState, BackBuffer);
}