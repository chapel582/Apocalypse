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

#define RESOURCE_TEXT_HEIGHT 15.0f
#define RESOURCE_LEFT_PADDING 100.0f

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

void RemoveCardAndAlign(card_game_state* SceneState, card* Card)
{
	card_set* CardSet = NULL;
	if(Card->SetType == CardSet_Hand)
	{
		CardSet = SceneState->Hands + Card->Owner;
	}
	else
	{
		CardSet = SceneState->Tableaus + Card->Owner; 
	}

	RemoveCardAndAlign(CardSet, Card);
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
	Card->Visible = true;
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
	card_definition* Definition = CardToDraw->Definition;
	Card->Definition = Definition;

	Card->TapsAvailable = Definition->TapsAvailable;
	Card->Attack = Definition->Attack;
	Card->TurnStartAttack = Definition->Attack;
	Card->Health = Definition->Health;
	Card->TurnStartHealth = Definition->Health;
	Card->TableauTags = Definition->TableauTags;
	Card->StackTags = Definition->StackTags;
	
	for(
		int PlayerIndex = 0;
		PlayerIndex < Player_Count;
		PlayerIndex++
	)
	{
		Card->PlayDelta[PlayerIndex] = Definition->PlayDelta[PlayerIndex];
		Card->TapDelta[PlayerIndex] = Definition->TapDelta[PlayerIndex];
		Card->TurnStartPlayDelta[PlayerIndex] = Definition->PlayDelta[PlayerIndex];
		Card->TurnStartTapDelta[PlayerIndex] = Definition->TapDelta[PlayerIndex];
	}
	Card->Owner = Owner;
	InDeckToOutDeck(Deck, CardToDraw);
}

card* DrawCard(
	game_state* GameState, card_game_state* SceneState, player_id Owner
)
{
	// NOTE: can't exceed maximum hand size
	card_set* CardSet = &SceneState->Hands[Owner];
	if(CardSet->CardCount >= MAX_CARDS_PER_SET)
	{
		DisplayMessageFor(
			GameState, &SceneState->Alert, "Can't draw card. Hand full", 1.0f
		);
		return NULL;
	}
	deck* Deck = &SceneState->Decks[Owner];
	
	card* Card = GetInactiveCard(SceneState);
	InitCardWithDeckCard(Deck, Card, Owner);
	AddCardAndAlign(CardSet, Card);
	return Card;
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

float GetTimeChangeFromCard(card* Card, player_resources* Delta)
{
	int32_t SelfResourceDelta = SumResources(Delta + RelativePlayer_Self);
	int32_t OppResourceDelta = SumResources(Delta + RelativePlayer_Opp);
	float TimeChange = (
		RESOURCE_TO_TIME * (SelfResourceDelta - OppResourceDelta)
	);
	return TimeChange;
}

bool CheckAndTapLand(
	game_state* GameState, card_game_state* SceneState, card* Card
)
{
	bool Tapped = false;

	float TimeChange = GetTimeChangeFromCard(Card, Card->TapDelta);
	
	if(TimeChange <= SceneState->TurnTimer)
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
	player_resources* PlayerResources = SceneState->PlayerResources;
	player_resources* Deltas = Card->TapDelta;
	
	// NOTE: only activate card if you have the resources for it
	player_resources* ChangeTarget = &PlayerResources[Card->Owner];
	player_resources* Delta = &Deltas[RelativePlayer_Self];
	bool Tapped = CanChangeResources(ChangeTarget, Delta);
	if(Tapped)
	{
		tableau_effect_tags* TableauTags = &Card->TableauTags;
		if(HasTag(TableauTags, TableauEffect_Land))
		{
			Tapped = CheckAndTapLand(GameState, SceneState, Card);
		}
		if(HasTag(TableauTags, TableauEffect_DrawExtra))
		{
			card* DrawnCard = DrawCard(GameState, SceneState, Card->Owner);
			if(DrawnCard != NULL)
			{
				float TimeChange = GetTimeChangeFromCard(
					DrawnCard, DrawnCard->PlayDelta
				);

				SceneState->TurnTimer += TimeChange;
				Tapped = true;
			}
		}
		if(HasTag(TableauTags, TableauEffect_DrawOppExtra))
		{
			card* DrawnCard = DrawCard(
				GameState, SceneState, GetOpponent(Card->Owner)
			);
			if(DrawnCard != NULL)
			{
				float TimeChange = GetTimeChangeFromCard(
					DrawnCard, DrawnCard->PlayDelta
				);

				SceneState->NextTurnTimer += TimeChange;
				Tapped = true;
			}
		}

		if(Tapped)
		{
			if(HasTag(TableauTags, TableauEffect_GetTime))
			{
				int32_t SelfResourceDelta = SumResources(
					Card->TapDelta + RelativePlayer_Self
				);
				float TimeChange = RESOURCE_TO_TIME * SelfResourceDelta;

				SceneState->TurnTimer += TimeChange;
			}
			else
			{
				ChangeResources(ChangeTarget, Delta);
			}

			player_id Opp = GetOpponent(Card->Owner);
			ChangeTarget = &PlayerResources[Opp];
			Delta = &Deltas[RelativePlayer_Opp];
			
			if(HasTag(TableauTags, TableauEffect_GiveTime))
			{
				int32_t OppResourceDelta = SumResources(
					Card->TapDelta + RelativePlayer_Opp
				);
				float TimeChange = RESOURCE_TO_TIME * OppResourceDelta;

				SceneState->NextTurnTimer += TimeChange;
			}
			else
			{
				ChangeResources(ChangeTarget, Delta);
			}

			if(HasTag(TableauTags, TableauEffect_TimeGrowth))
			{
				int32_t SelfResourceDelta = SumResources(
					Card->PlayDelta + RelativePlayer_Self
				);
				float TimeChange = RESOURCE_TO_TIME * SelfResourceDelta;

				SceneState->TurnTimer += TimeChange;
				card_set* Tableau = SceneState->Tableaus + Card->Owner;
				RemoveCardAndAlign(Tableau, Card);
				card_set* Hand = SceneState->Hands + Card->Owner;
				AddCardAndAlign(Hand, Card);
			}
		}
	}
	Card->TimesTapped++;

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
	player_resources* Delta = &Deltas[RelativePlayer_Self];
	bool Played = CanChangeResources(ChangeTarget, Delta);
	if(Played)
	{		
		// NOTE: card effects on play can be added here
		
		if(Played)
		{
			ChangeResources(ChangeTarget, Delta);
			player_id Opp = GetOpponent(Card->Owner);
			ChangeTarget = &PlayerResources[Opp];
			Delta = &Deltas[RelativePlayer_Opp];
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

struct attack_card_result 
{
	bool AttackerDied;
	bool AttackedDied;
};

attack_card_result AttackCard(
	card_game_state* SceneState, card* AttackingCard, card* AttackedCard
)
{
	// NOTE: this currently assumes cards must attack from the tableau
	attack_card_result Result = {};

	int16_t AttackingCardHealthDelta = AttackedCard->Attack;
	int16_t AttackedCardHealthDelta = AttackingCard->Attack;
	AttackingCard->Health -= AttackingCardHealthDelta;
	AttackedCard->Health -= AttackedCardHealthDelta;

	if(AttackingCard->Health <= 0)
	{
		RemoveCardAndAlign(SceneState, AttackingCard);
		AttackingCard->Active = false;
		Result.AttackerDied = true;
	}
	else
	{
		AttackingCard->TurnStartHealth -= AttackingCardHealthDelta;
	}

	if(AttackedCard->Health <= 0)
	{
		RemoveCardAndAlign(SceneState, AttackedCard);
		AttackedCard->Active = false;
		Result.AttackedDied = true;
	}
	else
	{
		AttackedCard->TurnStartHealth -= AttackedCardHealthDelta;
	}

	return Result;
}

void SetTurnTimer(card_game_state* SceneState, float Value)
{
	SceneState->TurnTimer = Value;
	uint16_t IntTurnTimer = (uint16_t) SceneState->TurnTimer;
	SceneState->LastWholeSecond = IntTurnTimer;
}

void StartCardGamePrep(
	game_state* GameState,
	char* P1DeckName,
	char* P2DeckName,
	bool NetworkGame,
	bool IsLeader,
	platform_socket* ListenSocket,
	platform_socket* ConnectSocket
)
{
	start_card_game_args* SceneArgs = PushStruct(
		&GameState->SceneArgsArena, start_card_game_args
	);
	*SceneArgs = {};

	char Buffer[PLATFORM_MAX_PATH];
	FormatDeckPath(Buffer, sizeof(Buffer), P1DeckName);
	SceneArgs->P1Deck = LoadDeck(Buffer);
	FormatDeckPath(Buffer, sizeof(Buffer), P2DeckName);
	SceneArgs->P2Deck = LoadDeck(Buffer);

	if(NetworkGame)
	{
		SceneArgs->IsLeader = IsLeader;
		if(ListenSocket)
		{
			SceneArgs->ListenSocket = *ListenSocket;
		}
		if(ConnectSocket)
		{
			SceneArgs->ConnectSocket = *ConnectSocket;
		}
		SceneArgs->NetworkGame = true;
	}

	GameState->SceneArgs = SceneArgs;
	GameState->Scene = SceneType_CardGame; 
}

void StartCardGame(
	game_state* GameState, uint32_t WindowWidth, uint32_t WindowHeight
)
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
	SceneState->ScreenDimInWorld = TransformVectorToBasis(
		&GameState->WorldToCamera,
		Vector2(WindowWidth, WindowHeight)
	);
	vector2 ScreenDimInWorld = SceneState->ScreenDimInWorld;
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
	
	SceneState->InfoCardCenter = ScreenDimInWorld / 2.0f;

	vector2 ScaledInfoCardDim = 0.33f * Vector2(600.0f, 900.0f);
	SceneState->InfoCardXBound = Vector2(ScaledInfoCardDim.X, 0.0f);
	SceneState->InfoCardYBound = Vector2(0.0f, ScaledInfoCardDim.Y);

	DrawFullHand(SceneState, Player_One);
	DrawFullHand(SceneState, Player_Two);

	SetTurnTimer(SceneState, 20.0f);
	SceneState->BaselineNextTurnTimer = (
		SceneState->TurnTimer + TURN_TIMER_INCREASE
	);
	SceneState->ShouldUpdateBaseline = false;
	SceneState->NextTurnTimer = SceneState->BaselineNextTurnTimer;

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

	SceneState->PlayerLife[Player_One] = 100.0f;
	SceneState->PlayerLife[Player_Two] = 100.0f;

	InitUiContext(&SceneState->UiContext);
	ui_context* UiContext = &SceneState->UiContext;

	SceneState->StackEntryInfoDim = Vector2(90.0f, 30.0f);
	vector2 StackEntryInfoDim = SceneState->StackEntryInfoDim;
	scroll_bar* StackScrollBar = &SceneState->StackScrollBar;

	rectangle StackScrollBox = MakeRectangle(
		Vector2(0.0f, ScreenDimInWorld.Y / 3.0f),
		Vector2(StackEntryInfoDim.X, ScreenDimInWorld.Y / 3.0f)
	);
	uint32_t ScrollBoxClipIndex = AddClipRect(
		&GameState->RenderGroup, StackScrollBox
	);
	InitScrollBar(
		UiContext, StackScrollBar, 20.0f, StackScrollBox, ScrollBoxClipIndex
	);
	SceneState->StackYStart = GetTop(StackScrollBox);


	// NOTE: the padding here is based on the height of the resource text info
	// CONT: this is all kinda hard-coded until we have a good scheme for UI
	// CONT: and also resolution scaling
	float Padding = 95.0f;
	vector2 PlayerLifeRectDim = Vector2(90.0f, 30.0f);
	SceneState->PlayerLifeRects[Player_One] = MakeRectangle(
		Vector2(
			ScreenDimInWorld.X - RESOURCE_LEFT_PADDING,
			(ScreenDimInWorld.Y / 2.0f) - Padding - 2.0f
		),
		PlayerLifeRectDim
	);
	SceneState->PlayerLifeRects[Player_Two] = MakeRectangle(
		Vector2(
			ScreenDimInWorld.X - RESOURCE_LEFT_PADDING,
			(ScreenDimInWorld.Y / 2.0f) + Padding + RESOURCE_TEXT_HEIGHT + 2.0f
		),
		PlayerLifeRectDim
	);

	SceneState->NetworkGame = SceneArgs->NetworkGame;
	SceneState->IsLeader = SceneArgs->IsLeader;
	SceneState->ListenSocket = SceneArgs->ListenSocket;
	SceneState->ConnectSocket = SceneArgs->ConnectSocket;
	SceneState->LastFrame = 0;

	// TODO: remove me!
	// PlaySound(
	// 	&GameState->PlayingSoundList,
	// 	WavHandle_TestMusic,
	// 	&GameState->TransientArena
	// );
}

float GetStackEntriesHeight(card_game_state* SceneState)
{
	float Height = 0.0f;
	for(
		uint32_t EntryIndex = 0;
		EntryIndex < SceneState->StackSize;
		EntryIndex++
	)
	{
		Height += SceneState->StackEntryInfoDim.Y;
	}

	return Height;
}

void SetStackPositions(card_game_state* SceneState)
{
	float NewY = GetElementsYStart(
		&SceneState->StackScrollBar, GetStackEntriesHeight(SceneState)
	);
	SceneState->StackYStart = NewY;

	for(
		int32_t EntryIndex = SceneState->StackSize - 1;
		EntryIndex >= 0;
		EntryIndex--
	)
	{
		card_stack_entry* Entry = SceneState->Stack + EntryIndex;
		card* StackCard = Entry->Card;
		SetTop(&StackCard->Rectangle, NewY);
		NewY -= StackCard->Rectangle.Dim.Y + 10.0f;
	}
}

void SwitchStackTurns(game_state* GameState, card_game_state* SceneState)
{
	SceneState->StackTurn = GetOpponent(SceneState->StackTurn);

	char Buffer[64];
	if(SceneState->StackTurn == Player_One)
	{
		snprintf(
			Buffer,
			sizeof(Buffer),
			"P1 can build on the stack (hit enter to trigger stack)"
		);
	}
	else
	{
		snprintf(
			Buffer,
			sizeof(Buffer),
			"P2 can build on the stack (hit enter to trigger stack)"
		);
	}
	DisplayMessageFor(GameState, &SceneState->Alert, Buffer, 1.0f);
}

void AddCardToStack(card_game_state* SceneState, card* Card)
{
	// NOTE: this handles putting the card on the stack w/r/t UI only
	RemoveCardAndAlign(SceneState, Card);
	Card->Rectangle.Dim = SceneState->StackEntryInfoDim;
	Card->SetType = CardSet_Stack;
	Card->Rectangle.Min.X = SceneState->StackScrollBar.ScrollBox.Min.X;
	SetStackPositions(SceneState);

	UpdateScrollBarPosDim(
		&SceneState->StackScrollBar,
		SceneState->StackYStart,
		GetStackEntriesHeight(SceneState)
	);
}

void EndStackBuilding(card_game_state* SceneState)
{
	if(SceneState->StackBuilding)
	{
		bool IsDisabled = false;
		while(SceneState->StackSize > 0)
		{
			card_stack_entry* CardStackEntry = (
				SceneState->Stack + (SceneState->StackSize - 1)
			);
			card* Card = CardStackEntry->Card;
			stack_effect_tags* StackTags = &Card->StackTags; 
			if(!IsDisabled)
			{
				if(HasTag(StackTags, StackEffect_HurtOpp))
				{
					player_id ToHurt = CardStackEntry->PlayerTarget;
					SceneState->PlayerLife[ToHurt] -= 5.0f;
				}
				if(HasTag(StackTags, StackEffect_DisableNext))
				{
					IsDisabled = true;
				}
				else
				{
					IsDisabled = false;
				}
			}

			SceneState->StackSize--;
			Card->Active = false;
		}
		SceneState->StackBuilding = false;

		scroll_bar* StackScrollBar = &SceneState->StackScrollBar;
		StackScrollBar->Rect.Dim.Y = StackScrollBar->Trough.Dim.Y + 1.0f;
	}
}

void PushTurnTimer(
	card_game_state* SceneState,
	player_id Player,
	render_group* RenderGroup,
	assets* Assets,
	memory_arena* FrameArena
)
{
	char* PlayerIndicator = NULL;
	if(Player == Player_One)
	{
		PlayerIndicator = "P1";
	}
	else if(Player == Player_Two)
	{
		PlayerIndicator = "P2";
	}
	else
	{
		ASSERT(false);
	}

	int32_t TurnTimerCeil = 0;
	if(SceneState->StackBuilding)
	{
		if(SceneState->StackTurn == Player)
		{
			TurnTimerCeil = Int32Ceil(SceneState->TurnTimer);
		}
		else
		{
			TurnTimerCeil = Int32Ceil(SceneState->NextTurnTimer);
		}
	}
	else
	{
		if(SceneState->CurrentTurn == Player)
		{
			TurnTimerCeil = Int32Ceil(SceneState->TurnTimer);
		}
		else
		{
			TurnTimerCeil = Int32Ceil(SceneState->NextTurnTimer);
		}
	}

	uint32_t MaxTurnTimerCharacters = 10;
	char* TurnTimerString = PushArray(
		FrameArena, MaxTurnTimerCharacters, char
	);
	ASSERT(PlayerIndicator != NULL);
	snprintf(
		TurnTimerString,
		MaxTurnTimerCharacters,
		"%s: %d",
		PlayerIndicator,
		TurnTimerCeil
	);
	PushText(
		RenderGroup,
		Assets,
		FontHandle_TestFont,
		TurnTimerString,
		MaxTurnTimerCharacters,
		50.0f,
		Vector2(
			SceneState->ScreenDimInWorld.X - 150.0f,
			SceneState->Tableaus[Player].YPos
		),
		Vector4(1.0f, 1.0f, 1.0f, 1.0f),
		FrameArena
	);
}

inline void StandardPrimaryUpHandler(
	game_state* GameState,
	card_game_state* SceneState,
	vector2 MouseEventWorldPos
)
{
	// NOTE: inlined function b/c this is only called once
	card* Card = &SceneState->Cards[0];

	for(int CardIndex = 0; CardIndex < SceneState->MaxCards; CardIndex++)
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
					if(SelectedCard == NULL)
					{
						bool WasPlayed = CheckAndPlay(
							GameState, SceneState, Card
						);
						if(WasPlayed)
						{
							if(HasAnyTag(&Card->StackTags))
							{
								card_stack_entry* StackEntry = (
									SceneState->Stack + SceneState->StackSize
								);
								StackEntry->Card = Card;
								SceneState->StackSize++;
								SceneState->StackBuilding = true;

								if(
									HasTag(
										&Card->StackTags, StackEffect_HurtOpp
									)
								)
								{
									StackEntry->PlayerTarget = (
										GetOpponent(SceneState->CurrentTurn)
									);
								}
								SceneState->StackTurn = Player_One;
								AddCardToStack(SceneState, Card);
								SwitchStackTurns(GameState, SceneState);
							}
							else
							{
								RemoveCardAndAlign(SceneState, Card);
								AddCardAndAlign(
									&SceneState->Tableaus[Card->Owner], Card
								);
							}
						}
						else
						{
							CannotActivateCardMessage(
								GameState, &SceneState->Alert
							);
						}
					}

					else
					{
						bool Tapped = CheckAndTap(
							GameState, SceneState, SelectedCard
						);
						if(Tapped)
						{
							DeselectCard(SceneState);
							float TimeChange = (
								GetTimeChangeFromCard(Card, Card->PlayDelta)
							);

							bool HasSelfBurn = HasTag(
								&SelectedCard->TableauTags, 
								TableauEffect_SelfBurn
							);
							attack_card_result Result = AttackCard(
								SceneState, SelectedCard, Card
							);
							if(Result.AttackedDied)
							{
								if(HasSelfBurn)
								{
									/*NOTE: 
									TimeChange is subtracted because
									we want the opposite effect from
									if it was played. If it would 
									cost the player time to play it,
									it now gives time. And vice 
									versa 
									*/
									SceneState->TurnTimer -= TimeChange;
								}
							}
						}
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
							tableau_effect_tags* Tags = (
								&Card->TableauTags
							);
							if(
								HasTag(Tags, TableauEffect_Land) ||
								HasTag(Tags, TableauEffect_DrawExtra) ||
								HasTag(Tags, TableauEffect_DrawOppExtra) ||
								HasTag(Tags, TableauEffect_TimeGrowth) ||
								HasTag(Tags, TableauEffect_AttackTimer)
							)
							{
								player_id Owner = SelectedCard->Owner;
								DeselectCard(SceneState);
								CheckAndTap(
									GameState, SceneState, SelectedCard
								);

								if(HasTag(Tags, TableauEffect_AttackTimer))
								{
									SceneState->NextTurnTimer -= Card->Attack;
								}
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
				if(SelectedCard != NULL)
				{
					if(Card->SetType == CardSet_Tableau)
					{
						player_id Owner = SelectedCard->Owner;
						bool Tapped = CheckAndTap(
							GameState,
							SceneState,
							SelectedCard
						);
						if(Tapped)
						{
							DeselectCard(SceneState);
							AttackCard(SceneState, SelectedCard, Card);
						}
					}
					else if(
						Card->SetType == CardSet_Hand && 
						HasTag(
							&SelectedCard->TableauTags, TableauEffect_OppBurn
						)									
					)
					{
						bool Tapped = CheckAndTap(
							GameState, SceneState, SelectedCard
						);
						if(Tapped)
						{
							DeselectCard(SceneState);
							float TimeChange = (
								GetTimeChangeFromCard(Card, Card->PlayDelta)
							);

							attack_card_result Result = AttackCard(
								SceneState, SelectedCard, Card
							);
							if(Result.AttackedDied)
							{
								SceneState->NextTurnTimer -= TimeChange;
							}
						}
					}
				}
			}
		}
		Card++;
	}

	for(int Player = Player_One; Player < Player_Count; Player++)
	{
		if(
			PointInRectangle(
				MouseEventWorldPos, SceneState->PlayerLifeRects[Player]
			)
		)
		{
			card* SelectedCard = SceneState->SelectedCard;
			if(SelectedCard != NULL)
			{
				// TODO: give a confirmation option for attacking yourself
				player_id Owner = SelectedCard->Owner;
				bool Tapped = CheckAndTap(
					GameState,
					SceneState,
					SelectedCard
				);
				if(Tapped)
				{
					DeselectCard(SceneState);
					SceneState->PlayerLife[Player] -= SelectedCard->Attack;

					if(SceneState->PlayerLife[Player] <= 0.0f)
					{
						// TODO: give a small amount of fanfare for the winner
						GameState->Scene = SceneType_MainMenu;
					}
				}
			}
		}
	}
}

inline void StackBuildingPrimaryUpHandler(
	game_state* GameState,
	card_game_state* SceneState,
	vector2 MouseEventWorldPos
)
{
	card* Card = &SceneState->Cards[0];

	for(int CardIndex = 0; CardIndex < SceneState->MaxCards; CardIndex++)
	{
		if(
			Card->Active &&
			PointInRectangle(MouseEventWorldPos, Card->Rectangle)
		)
		{
			card* SelectedCard = SceneState->SelectedCard;

			// NOTE: player clicked their own card on their turn 
			if(Card->Owner == SceneState->StackTurn)
			{
				if(Card->SetType == CardSet_Hand)
				{
					bool WasPlayed = CheckAndPlay(GameState, SceneState, Card);
					if(WasPlayed)
					{
						if(HasAnyTag(&Card->StackTags))
						{
							card_stack_entry* StackEntry = (
								SceneState->Stack + SceneState->StackSize
							);
							StackEntry->Card = Card;
							if(HasTag(&Card->StackTags, StackEffect_HurtOpp))
							{
								StackEntry->PlayerTarget = GetOpponent(
									SceneState->StackTurn
								);
							}
							SceneState->StackSize++;

							AddCardToStack(SceneState, Card);
							SwitchStackTurns(GameState, SceneState);
						}
						else
						{
							DisplayMessageFor(
								GameState,
								&SceneState->Alert,
								"Card is not a stack card",
								1.0f
							);
						}
					}
					else
					{
						CannotActivateCardMessage(
							GameState, &SceneState->Alert
						);
					}
				}
				break;
			}
			// NOTE: player clicked on opponent's card on their turn
			else
			{
			}
		}
		Card++;
	}
}

inline bool StandardKeyboardHandler(
	game_state* GameState,
	card_game_state* SceneState,
	game_keyboard_event* KeyboardEvent
)
{
	// NOTE: end turn currently is the return value
	bool EndTurn = false;

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
		}
	}
	return EndTurn;
}

inline bool StackBuildingKeyboardHandler(
	game_state* GameState,
	card_game_state* SceneState,
	game_keyboard_event* KeyboardEvent
)
{
	// NOTE: end turn is the current return type
	bool EndTurn = false;

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
					GameState->OverlayDebugInfo = !GameState->OverlayDebugInfo;
				}
				break;
			}
		}
	}

	return EndTurn;
}

void UpdateAndRenderCardGame(
	game_state* GameState,
	card_game_state* SceneState,
	uint32_t WindowWidth,
	uint32_t WindowHeight,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
)
{
	ui_context* UiContext = &SceneState->UiContext;
	memory_arena* FrameArena = &GameState->FrameArena;
	assets* Assets = &GameState->Assets;

	SceneState->ScreenDimInWorld = TransformVectorToBasis(
		&GameState->WorldToCamera,
		Vector2(WindowWidth, WindowHeight)
	);
	vector2 ScreenDimInWorld = SceneState->ScreenDimInWorld;

	bool EndTurn = false;

	// SECTION START: Read leader state
	if(SceneState->NetworkGame && !SceneState->IsLeader)
	{
		// TODO: handle packets coming in pieces
		packet_header* Header = PushStruct(FrameArena, packet_header);

		uint32_t BytesRead = 0;
		while(true)
		{
			platform_read_socket_result SocketReadResult = PlatformSocketRead(
				&SceneState->ConnectSocket,
				Header,
				sizeof(packet_header),
				&BytesRead
			);
			if(
				SocketReadResult == PlatformReadSocketResult_Success && 
				BytesRead > 0
			)
			{
				// NOTE: read the rest of our packet from the socket buffer
				uint32_t DataRemaining = (
					Header->DataSize - sizeof(packet_header)
				);
				void* Payload = PushSize(FrameArena, DataRemaining);
				SocketReadResult = PlatformSocketRead(
					&SceneState->ConnectSocket,
					Payload,
					DataRemaining,
					&BytesRead
				);
				ASSERT(
					SocketReadResult ==
					PlatformReadSocketResult_Success
				);
				ASSERT(BytesRead > 0);

				// NOTE: check that the frame is a new frame to handle
				if(Header->FrameId > SceneState->LastFrame)
				{
					SceneState->LastFrame = Header->FrameId;
					switch(Header->Type)
					{
						case(Packet_StateUpdate):
						{
							state_update_payload* LeaderState = (
								(state_update_payload*) Payload
							);
							SceneState->TurnTimer = LeaderState->TurnTimer;
							break;
						}
						default:
						{
							// TODO: logging
							ASSERT(false);
						}
					}
				}
			}
			else
			{
				break;
			}
		} while (BytesRead > 0); 
	}
	// SECTION STOP: Read leader state

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
				// PlaySound(
				// 	&GameState->PlayingSoundList,
				// 	WavHandle_Bloop00,
				// 	&GameState->TransientArena
				// );
				if(SceneState->StackBuilding)
				{
					StackBuildingPrimaryUpHandler(
						GameState, SceneState, MouseEventWorldPos
					);
				}
				else
				{
					StandardPrimaryUpHandler(
						GameState, SceneState, MouseEventWorldPos
					);
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

			scroll_handle_mouse_code ScrollResult = ScrollHandleMouse(
				UiContext,
				&SceneState->StackScrollBar,
				MouseEvent,
				MouseEventWorldPos,
				SceneState->StackScrollBar.Trough.Min.Y,
				GetTop(SceneState->StackScrollBar.Trough)
			);
			if(ScrollResult == ScrollHandleMouse_Moved)
			{
				SetStackPositions(SceneState);
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

			bool TempEndTurn = false;
			if(!SceneState->StackBuilding)
			{
				TempEndTurn = StandardKeyboardHandler(
					GameState, SceneState, KeyboardEvent
				);
				// NOTE: this is not unnecessary. Don't want to overwrite EndTurn 
				// CONT: with false by accident
				if(TempEndTurn)
				{
					EndTurn = TempEndTurn;
				}
			}
			else
			{
				TempEndTurn = StackBuildingKeyboardHandler(
					GameState, SceneState, KeyboardEvent
				);
				if(TempEndTurn)
				{
					EndStackBuilding(SceneState);
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
		if(SceneState->StackBuilding)
		{
			if(SceneState->StackTurn == SceneState->CurrentTurn)
			{
				SceneState->TurnTimer -= DtForFrame;
			}
			else
			{
				SceneState->NextTurnTimer -= DtForFrame;
				if(SceneState->NextTurnTimer <= 0.0f)
				{
					SceneState->NextTurnTimer = 0.0f;
				}

				if(SceneState->NextTurnTimer <= 0.0f)
				{
					EndStackBuilding(SceneState);
				}
			}
		}
		else
		{
			SceneState->TurnTimer -= DtForFrame;
		}
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
		EndStackBuilding(SceneState);

		SetTurnTimer(SceneState, SceneState->NextTurnTimer);

		if(SceneState->ShouldUpdateBaseline)
		{
			SceneState->BaselineNextTurnTimer += TURN_TIMER_INCREASE;
			SceneState->ShouldUpdateBaseline = false;
		}
		else
		{
			SceneState->ShouldUpdateBaseline = true;
		}
		SceneState->NextTurnTimer = SceneState->BaselineNextTurnTimer;

		SceneState->CurrentTurn = (
			(SceneState->CurrentTurn == Player_Two) ? Player_One : Player_Two
		);
		for(int CardIndex = 0; CardIndex < SceneState->MaxCards; CardIndex++)
		{
			card* Card = &SceneState->Cards[CardIndex];
			if(Card->Active)
			{
				Card->TimesTapped = 0;

				tableau_effect_tags* EffectTags = &Card->TableauTags;
				if(HasTag(EffectTags, TableauEffect_SelfWeaken))
				{
					if(Card->SetType == CardSet_Tableau)
					{
						Card->Attack = Card->TurnStartAttack;
					}
				}
				if(HasTag(EffectTags, TableauEffect_SelfHandWeaken))
				{
					if(Card->SetType == CardSet_Hand)
					{
						Card->Attack = Card->TurnStartAttack;
					}
				}
				if(HasTag(EffectTags, TableauEffect_OppStrengthen))
				{
					if(Card->SetType == CardSet_Tableau)
					{
						Card->Attack = Card->TurnStartAttack;
					}
				}
				if(HasTag(EffectTags, TableauEffect_SelfLifeLoss))
				{
					if(Card->SetType == CardSet_Tableau)
					{
						Card->Health = Card->TurnStartHealth;
					}
				}
				if(HasTag(EffectTags, TableauEffect_OppLifeGain))
				{
					if(Card->SetType == CardSet_Tableau)
					{
						Card->Health = Card->TurnStartHealth;
					}
				}
				if(HasTag(EffectTags, TableauEffect_CostIncrease))
				{
					if(Card->SetType == CardSet_Hand)
					{
						Card->PlayDelta[RelativePlayer_Self] = (
							Card->TurnStartPlayDelta[RelativePlayer_Self]
						);
					}
				}
				if(HasTag(EffectTags, TableauEffect_GiveIncrease))
				{
					if(Card->SetType == CardSet_Hand)
					{
						Card->PlayDelta[RelativePlayer_Opp] = (
							Card->TurnStartPlayDelta[RelativePlayer_Opp]
						);
					}
				}
				if(HasTag(EffectTags, TableauEffect_TimeGrowth))
				{
					player_resources* PlayDelta = (
						Card->PlayDelta + RelativePlayer_Self
					);
					for(
						int Resource = 0;
						Resource < PlayerResource_Count;
						Resource++
					)
					{
						PlayDelta->Resources[Resource] += 1;
					}
				}
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

				tableau_effect_tags* TableauTags = &Card->TableauTags;
				if(HasTag(TableauTags, TableauEffect_SelfWeaken))
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
				if(HasTag(TableauTags, TableauEffect_SelfHandWeaken))
				{
					if(
						Card->Owner == SceneState->CurrentTurn && 
						Card->SetType == CardSet_Hand
					)
					{
						if(WholeSecondPassed)
						{
							Card->Attack -= 1;
						}
					}
				}
				if(HasTag(TableauTags, TableauEffect_OppStrengthen))
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
				if(HasTag(TableauTags, TableauEffect_SelfLifeLoss))
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
							RemoveCardAndAlign(SceneState, Card);
							Card->Active = false;
						}
					}
				}
				if(HasTag(TableauTags, TableauEffect_OppLifeGain))
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
				if(HasTag(TableauTags, TableauEffect_CostIncrease))
				{
					if(Card->SetType == CardSet_Hand)
					{
						if(WholeSecondPassed)
						{
							for(
								int Index = 0;
								Index < PlayerResource_Count;
								Index++
							)
							{
								int32_t* Resource = (
									&Card->PlayDelta[RelativePlayer_Self].Resources[Index]
								);
								if(*Resource < 0)
								{
									*Resource -= 1;
								}
							}
						}
					}
				}
				if(HasTag(TableauTags, TableauEffect_GiveIncrease))
				{
					if(Card->SetType == CardSet_Hand)
					{
						if(WholeSecondPassed)
						{
							int32_t* Resources = (
								Card->PlayDelta[RelativePlayer_Opp].Resources
							);
							for(
								int Index = 0;
								Index < PlayerResource_Count;
								Index++
							)
							{
								int32_t* Resource = Resources + Index;
								if(*Resource > 0)
								{
									*Resource += 1;
								}
							}
						}
					}
				}
			}
			Card++;
		}
	}
	// SECTION STOP: Card update
	// SECTION STOP: Updating game state

	// SECTION START: Send data to follower
	if(SceneState->NetworkGame && SceneState->IsLeader)
	{
		state_update_packet* StatePacket = PushStruct(
			FrameArena, state_update_packet
		);
		packet_header* Header = &StatePacket->Header;
		Header->Type = Packet_StateUpdate;
		Header->DataSize = sizeof(state_update_packet);
		Header->FrameId = GameState->FrameCount;
		state_update_payload* Payload = &StatePacket->Payload;
		Payload->TurnTimer = SceneState->TurnTimer;

		socket_send_data_args* SendStatePacketArgs = PushStruct(
			FrameArena, socket_send_data_args
		);
		SendStatePacketArgs->Socket = &SceneState->ConnectSocket;
		SendStatePacketArgs->Buffer = StatePacket;
		SendStatePacketArgs->BufferSize = Header->DataSize;
		SendStatePacketArgs->DataSize = Header->DataSize;
		PlatformAddJob(
			GameState->JobQueue,
			SocketSendDataJob,
			SendStatePacketArgs,
			JobPriority_SendPacket
		);
	}
	// SECTION STOP: Send data to follower

	// SECTION START: Push render entries
	render_group* RenderGroup = &GameState->RenderGroup;
	PushClear(RenderGroup, Vector4(0.25f, 0.25f, 0.25f, 1.0f));

	PushTurnTimer(SceneState, Player_One, RenderGroup, Assets, FrameArena);
	PushTurnTimer(SceneState, Player_Two, RenderGroup, Assets, FrameArena);

	// SECTION START: Push cards
	{
		card* Card = &SceneState->Cards[0];
		for(
			int CardIndex = 0;
			CardIndex < SceneState->MaxCards;
			CardIndex++
		)
		{
			if(Card->Active && Card->Visible)
			{
				if(Card->SetType != CardSet_Stack)
				{
					vector2 Center = GetCenter(Card->Rectangle); 
					PushSizedBitmap(
						RenderGroup,
						Assets,
						BitmapHandle_TestCard2,
						Center,
						Vector2(Card->Rectangle.Dim.X, 0.0f),
						Vector2(0.0f, Card->Rectangle.Dim.Y),
						Card->Color,
						1
					);
					PushTextCentered(
						RenderGroup,
						Assets,
						FontHandle_TestFont,
						Card->Definition->Name,
						CARD_NAME_SIZE,
						0.2f * Card->Rectangle.Dim.Y, 
						Center,
						Vector4(0, 0, 0, 1),
						FrameArena,
						2
					);
				}
				else
				{
					vector2 Center = GetCenter(Card->Rectangle);
					PushSizedBitmap(
						RenderGroup,
						Assets,
						BitmapHandle_TestCard2,
						Center,
						Vector2(Card->Rectangle.Dim.X, 0.0f),
						Vector2(0.0f, Card->Rectangle.Dim.Y),
						Card->Color,
						1,
						SceneState->StackScrollBar.ScrollBoxClipIndex
					);
					PushTextCentered(
						RenderGroup,
						Assets,
						FontHandle_TestFont,
						Card->Definition->Name,
						CARD_NAME_SIZE,
						0.8f * Card->Rectangle.Dim.Y, 
						Center,
						Vector4(0, 0, 0, 1),
						FrameArena,
						2
					);
				}
				if(Card->HoveredOver)
				{
					PushInfoCard(
						RenderGroup,
						Assets,
						SceneState->InfoCardCenter,
						SceneState->InfoCardXBound,
						SceneState->InfoCardYBound,
						Card->Color,
						FrameArena,
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
			FrameArena, MAX_RESOURCE_STRING_SIZE, char
		);
		FormatResourceString(
			ResourceString, &SceneState->PlayerResources[Player_One]
		);
		float Padding = 15.0f;
		PushText(
			RenderGroup,
			Assets,
			FontHandle_TestFont,
			ResourceString,
			MAX_RESOURCE_STRING_SIZE,
			RESOURCE_TEXT_HEIGHT,
			Vector2(
				ScreenDimInWorld.X - RESOURCE_LEFT_PADDING,
				(ScreenDimInWorld.Y / 2.0f) - Padding
			),
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			FrameArena
		);

		FormatResourceString(
			ResourceString, &SceneState->PlayerResources[Player_Two]
		);
		PushText(
			RenderGroup,
			Assets,
			FontHandle_TestFont,
			ResourceString,
			MAX_RESOURCE_STRING_SIZE,
			RESOURCE_TEXT_HEIGHT,
			Vector2(
				ScreenDimInWorld.X - RESOURCE_LEFT_PADDING,
				(ScreenDimInWorld.Y / 2.0f) + 80.0f + Padding
			),
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			FrameArena
		);
	}
	// SECTION STOP: Push resources

	// SECTION START: Push player life totals
	{
		#define MAX_PLAYER_LIFE_STRING 16
		char* PlayerLifeString = PushArray(
			FrameArena, MAX_PLAYER_LIFE_STRING, char
		);
		snprintf(
			PlayerLifeString,
			MAX_RESOURCE_STRING_SIZE,
			"P1:%d",
			(int) SceneState->PlayerLife[Player_One]
		);

		vector4 White = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		vector4 Black = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		float Padding = 95.0f;
		rectangle* PlayerLifeRects = SceneState->PlayerLifeRects;
		PushSizedBitmap(
			RenderGroup,
			Assets,
			BitmapHandle_TestCard2,
			GetCenter(PlayerLifeRects[Player_One]),
			Vector2(PlayerLifeRects[Player_One].Dim.X, 0.0f),
			Vector2(0.0f, PlayerLifeRects[Player_One].Dim.Y),
			White,
			1
		);
		PushText(
			RenderGroup,
			Assets,
			FontHandle_TestFont,
			PlayerLifeString,
			MAX_RESOURCE_STRING_SIZE,
			PlayerLifeRects[Player_One].Dim.Y,
			PlayerLifeRects[Player_One].Min,
			Black,
			FrameArena,
			2
		);

		snprintf(
			PlayerLifeString,
			MAX_RESOURCE_STRING_SIZE,
			"P2:%d",
			(int) SceneState->PlayerLife[Player_Two]
		);
		PushSizedBitmap(
			RenderGroup,
			Assets,
			BitmapHandle_TestCard2,
			GetCenter(PlayerLifeRects[Player_Two]),
			Vector2(PlayerLifeRects[Player_Two].Dim.X, 0.0f),
			Vector2(0.0f, PlayerLifeRects[Player_Two].Dim.Y),
			White,
			1
		);
		PushText(
			RenderGroup,
			Assets,
			FontHandle_TestFont,
			PlayerLifeString,
			MAX_RESOURCE_STRING_SIZE,
			PlayerLifeRects[Player_Two].Dim.Y,
			PlayerLifeRects[Player_Two].Min,
			Black,
			FrameArena,
			2
		);
	}
	// SECTION STOP: Push player life totals

	PushCenteredAlert(&SceneState->Alert, GameState, ScreenDimInWorld);

	if(CanScroll(&SceneState->StackScrollBar))
	{
		PushScrollBarToRenderGroup(
			SceneState->StackScrollBar.Rect,
			BitmapHandle_TestCard2,
			RenderGroup,
			Assets
		);
	}

	PlatformCompleteAllJobsAtPriority(
		GameState->JobQueue, JobPriority_SendPacket
	);
}