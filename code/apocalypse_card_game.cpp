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
#include "apocalypse_packet_header.h"

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

void RemoveCardFromStack(card_game_state* SceneState, card* Card)
{
	card_stack_entry* FoundAt = NULL;
	for(
		uint32_t StackIndex = 0;
		StackIndex < SceneState->StackSize;
		StackIndex++
	)
	{
		card_stack_entry* CardStackEntry = SceneState->Stack + StackIndex;
		card* CardToCheck = CardStackEntry->Card;
		if(CardToCheck == Card)
		{
			FoundAt = CardStackEntry;
			break;
		}
	}
	ASSERT(FoundAt != NULL);
	SceneState->StackSize--;
	memcpy(
		FoundAt,
		FoundAt + 1,
		(
			(SceneState->StackSize - (FoundAt - SceneState->Stack)) * 
			sizeof(card_stack_entry)
		)
	);
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

bool RemoveCardFromSet(card_game_state* SceneState, card* Card)
{	
	card_set* CardSet = NULL;	
	switch(Card->SetType)
	{
		case(CardSet_Hand):
		{
			CardSet = SceneState->Hands + Card->Owner;
			break;
		}
		case(CardSet_Tableau):
		{
			CardSet = SceneState->Tableaus + Card->Owner;
			break;
		}
		case(CardSet_Stack):
		{
			// NOTE: different flow for stack
			RemoveCardFromStack(SceneState, Card);
			return true;
		}
		default:
		{
			ASSERT(false);
			break;
		}
	}
	return RemoveCardFromSet(CardSet, Card);
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
	else if(Card->SetType == CardSet_Tableau)
	{
		CardSet = SceneState->Tableaus + Card->Owner; 
	}
	else
	{
		ASSERT(false);
	}

	RemoveCardAndAlign(CardSet, Card);
}

void SafeRemoveCardCommon(
	game_state* GameState, card_game_state* SceneState, card* Card
)
{
	Card->Active = false;
	if(SceneState->NetworkGame && SceneState->IsLeader)
	{
		memory_arena* FrameArena = &GameState->FrameArena;
		remove_card_packet* Packet = PushStruct(
			FrameArena, remove_card_packet
		);		
		remove_card_payload* Payload = &Packet->Payload;
		Payload->CardId = Card->CardId;

		packet_header* Header = &Packet->Header;
		Header->DataSize = sizeof(remove_card_packet);
		InitPacketHeader(
			GameState, Header, Packet_RemoveCard, (uint8_t*) Payload
		);

		SocketSendErrorCheck(
			GameState,
			&SceneState->ConnectSocket,
			&SceneState->ListenSocket,
			Header
		);
	}
}

void SafeRemoveCard(
	game_state* GameState, card_game_state* SceneState, card* Card
)
{
	// NOTE: a function for removing the card from its card set 
	// CONT: without any concerns for speed. Card will no longer be active and 
	// CONT: the card set will be decremented but not aligned
	if(Card->SetType == CardSet_Stack)
	{
		RemoveCardFromStack(SceneState, Card);
	}
	else
	{
		RemoveCardFromSet(SceneState, Card);
	}
	SafeRemoveCardCommon(GameState, SceneState, Card);
}

void SafeRemoveCardAndAlign(
	game_state* GameState, card_game_state* SceneState, card* Card
)
{
	// NOTE: a function for removing the card from its card set 
	// CONT: without any concerns for speed. Card will no longer be active and 
	// CONT: the card set will be decremented and aligned
	if(Card->SetType == CardSet_Stack)
	{
		RemoveCardFromStack(SceneState, Card);
	}
	else
	{
		RemoveCardAndAlign(SceneState, Card);
	}
	SafeRemoveCardCommon(GameState, SceneState, Card);
}

card* GetInactiveCard(card_game_state* SceneState, int32_t CardId = -1)
{
	card* Card = NULL;
	for(
		uint32_t SearchIndex = 0;
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

	*Card = {};
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
	if(CardId == -1)
	{
		Card->CardId = SceneState->NextCardId++;
	}
	else
	{
		Card->CardId = CardId;
	}
	return Card;
}

void InitCardWithDef(
	card_game_state* SceneState, card* Card, card_definition* Definition
)
{
	Card->LastFrame = 0;
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
}
void InitCardWithDef(
	card_game_state* SceneState,
	card* Card,
	card_definitions* Definitions,
	uint32_t DefId,
	player_id Owner
)
{
	card_definition* Definition = GetCardDefinition(Definitions, DefId);
	InitCardWithDef(SceneState, Card, Definition);
	Card->Owner = Owner;
}

void InitCardWithDeckCard(
	card_game_state* SceneState, deck* Deck, card* Card, player_id Owner
)
{
	ASSERT(Deck->InDeckCount > 0);
	ASSERT(Deck->InDeck[0] < MAX_CARDS_IN_DECK);
	deck_card* CardToDraw = Deck->Cards + Deck->InDeck[0];

	// NOTE: remove the top card from the deck
	Deck->InDeckCount--;
	memcpy(
		Deck->InDeck, Deck->InDeck + 1, Deck->InDeckCount * sizeof(uint32_t)
	);

	card_definition* Definition = CardToDraw->Definition;

	InitCardWithDef(SceneState, Card, Definition);
	Card->Owner = Owner;
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
	InitCardWithDeckCard(SceneState, Deck, Card, Owner);
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
		InitCardWithDeckCard(SceneState, Deck, Card, Player);
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
		
		// TODO: see if this conditional is still needed
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

card_stack_entry* AddCardToStack(card_game_state* SceneState, card* Card)
{
	RemoveCardAndAlign(SceneState, Card);
	
	card_stack_entry* StackEntry = SceneState->Stack + SceneState->StackSize;
	StackEntry->Card = Card;
	SceneState->StackSize++;

	Card->Rectangle.Dim = SceneState->StackEntryInfoDim;
	Card->SetType = CardSet_Stack;
	Card->Rectangle.Min.X = SceneState->StackScrollBar.ScrollBox.Min.X;
	SetStackPositions(SceneState);

	UpdateScrollBarPosDim(
		&SceneState->StackScrollBar,
		SceneState->StackYStart,
		GetStackEntriesHeight(SceneState)
	);

	return StackEntry;
}

void SwitchTurns(game_state* GameState, card_game_state* SceneState)
{
	SceneState->CurrentTurn = GetOpponent(SceneState->CurrentTurn);
	char Buffer[64];
	if(SceneState->CurrentTurn == Player_One)
	{
		snprintf(Buffer, sizeof(Buffer), "P1's turn");
	}
	else
	{
		snprintf(Buffer, sizeof(Buffer), "P2's turn");
	}
	DisplayMessageFor(GameState, &SceneState->Alert, Buffer, 1.0f);
}

void SwitchStackTurns(game_state* GameState, card_game_state* SceneState)
{
	SceneState->StackTurn = GetOpponent(SceneState->StackTurn);

	char Buffer[64];
	if(!SceneState->NetworkGame)
	{
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
	}
	else
	{
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
				"P2 can build on the stack"
			);
		}
	}
	DisplayMessageFor(GameState, &SceneState->Alert, Buffer, 1.0f);
}

void PlayStackCard(
	game_state* GameState, card_game_state* SceneState, card* Card
)
{
	// NOTE: this function plays the stack card without checking HasAnyTag
	card_stack_entry* StackEntry = AddCardToStack(SceneState, Card);
	if(HasTag(&Card->StackTags, StackEffect_HurtOpp))
	{
		StackEntry->PlayerTarget = GetOpponent(SceneState->CurrentTurn);
	}
	
	SwitchStackTurns(GameState, SceneState);
}

void NormalPlayCard(
	game_state* GameState, card_game_state* SceneState, card* Card
)
{
	// NOTE: handle cards that go onto the stack
	if(HasAnyTag(&Card->StackTags))
	{
		SceneState->StackBuilding = true;
		SceneState->StackTurn = SceneState->CurrentTurn;
		PlayStackCard(GameState, SceneState, Card);
	}
	// NOTE: handle cards that go in the tableau
	else
	{
		RemoveCardAndAlign(SceneState, Card);
		AddCardAndAlign(&SceneState->Tableaus[Card->Owner], Card);
	}
}

bool StackBuildingPlayCard(
	game_state* GameState, card_game_state* SceneState, card* Card
)
{
	bool WasPlayed = false;
	if(HasAnyTag(&Card->StackTags))
	{
		PlayStackCard(GameState, SceneState, Card);
		WasPlayed = true;
	}

	return WasPlayed;
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
	game_state* GameState,
	card_game_state* SceneState,
	card* AttackingCard,
	card* AttackedCard
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
		SafeRemoveCardAndAlign(GameState, SceneState, AttackingCard);
		Result.AttackerDied = true;
	}
	else
	{
		AttackingCard->TurnStartHealth -= AttackingCardHealthDelta;
	}

	if(AttackedCard->Health <= 0)
	{
		SafeRemoveCardAndAlign(GameState, SceneState, AttackedCard);
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

start_card_game_args* StartCardGamePrepCommon(
	game_state* GameState,
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

	if(NetworkGame)
	{
		SceneArgs->IsLeader = IsLeader;
	
		SceneArgs->ListenSocket = {};
		SceneArgs->ConnectSocket = {};
		if(ListenSocket)
		{
			SceneArgs->ListenSocket = *ListenSocket;
			SceneArgs->ListenSocket.IsValid = true;
		}
		else
		{
			SceneArgs->ListenSocket.IsValid = false; 
		}
		if(ConnectSocket)
		{
			SceneArgs->ConnectSocket = *ConnectSocket;
			SceneArgs->ConnectSocket.IsValid = true;
		}
		else
		{
			SceneArgs->ConnectSocket.IsValid = false;
		}
		SceneArgs->NetworkGame = true;
	}

	GameState->SceneArgs = SceneArgs;
	GameState->Scene = SceneType_CardGame; 

	return SceneArgs;
}

void StartCardGamePrep(
	game_state* GameState,
	char* P1DeckName,
	char* P2DeckName,
	bool NetworkGame,
	bool IsLeader
)
{
	start_card_game_args* SceneArgs = StartCardGamePrepCommon(
		GameState,
		NetworkGame,
		IsLeader,
		NULL,
		NULL
	);

	char Buffer[PLATFORM_MAX_PATH];
	FormatDeckPath(Buffer, sizeof(Buffer), P1DeckName);
	SceneArgs->P1Deck = LoadDeck(Buffer);
	FormatDeckPath(Buffer, sizeof(Buffer), P2DeckName);
	SceneArgs->P2Deck = LoadDeck(Buffer);
}

void StartCardGamePrep(
	game_state* GameState,
	loaded_deck P1Deck,
	loaded_deck P2Deck,
	bool NetworkGame,
	bool IsLeader,
	platform_socket* ListenSocket,
	platform_socket* ConnectSocket
)
{
	start_card_game_args* SceneArgs = StartCardGamePrepCommon(
		GameState,
		NetworkGame,
		IsLeader,
		ListenSocket,
		ConnectSocket
	);

	SceneArgs->P1Deck = P1Deck;
	SceneArgs->P2Deck = P2Deck;
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
	
	// NOTE: set up game config stuff
	SceneState->NetworkGame = SceneArgs->NetworkGame;
	SceneState->IsLeader = SceneArgs->IsLeader;
	SceneState->ListenSocket = SceneArgs->ListenSocket;
	SceneState->ConnectSocket = SceneArgs->ConnectSocket;
	SceneState->LastFrame = 0;

	SceneState->NextCardId = 0;
	if(SceneState->NetworkGame && !SceneState->IsLeader)
	{
		SceneState->NextCardId = 1 << 16;
	}

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
			Deck->CardCount = LoadedDeck->Header.CardCount;

			deck_card* DeckCard = &Deck->Cards[0];
			*DeckCard = {};
			InitDeckCard(
				DeckCard, SceneState->Definitions, LoadedDeck->Ids[0]
			);
			for(
				int CardIndex = 1;
				CardIndex < LoadedDeck->Header.CardCount;
				CardIndex++
			)
			{
				DeckCard = &Deck->Cards[CardIndex];
				*DeckCard = {};
				InitDeckCard(
					DeckCard,
					SceneState->Definitions,
					LoadedDeck->Ids[CardIndex]
				);
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

			Deck->InDeckCount = 0;
			uint32_t OutOfDeckCount = Deck->CardCount - Deck->InDeckCount;
			while(OutOfDeckCount > 0)
			{
				int32_t ShuffleIndex = rand() % OutOfDeckCount;
				int32_t CardToShuffleHandle = -1;
				for(
					int32_t CardIndex = 0;
					CardIndex < ((int32_t) Deck->CardCount);
					CardIndex++
				)
				{
					deck_card* DeckCard = Deck->Cards + CardIndex;
					if(!DeckCard->InDeck)
					{
						ShuffleIndex--;
					}

					if(ShuffleIndex < 0)
					{
						CardToShuffleHandle = CardIndex;
						break;
					}
				}
				ASSERT(CardToShuffleHandle != -1);
				Deck->InDeck[Deck->InDeckCount] = CardToShuffleHandle;
				Deck->InDeckCount++;
				OutOfDeckCount = Deck->CardCount - Deck->InDeckCount;
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

	// TODO: this may not be a great way to seed. might be exploitable?
	uint32_t Seed = (uint32_t) time(NULL);
	srand(Seed);
	if(SceneState->NetworkGame && SceneState->IsLeader)
	{
		memory_arena* FrameArena = &GameState->FrameArena;
		rand_seed_packet* RandSeedPacket = PushStruct(
			FrameArena, rand_seed_packet
		);
		rand_seed_payload* Payload = &RandSeedPacket->Payload;
		Payload->Seed = Seed;

		packet_header* Header = &RandSeedPacket->Header; 
		Header->DataSize = sizeof(rand_seed_packet);
		InitPacketHeader(
			GameState, Header, Packet_RandSeed, (uint8_t*) Payload
		);
		SocketSendErrorCheck(
			GameState,
			&SceneState->ConnectSocket,
			&SceneState->ListenSocket,
			Header
		);
	}

	if(
		!SceneState->NetworkGame || 
		(SceneState->NetworkGame && SceneState->IsLeader)
	)
	{
		DrawFullHand(SceneState, Player_One);
		DrawFullHand(SceneState, Player_Two);
	}

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

	SceneState->PacketReader = {};
	SceneState->PacketReader.NetworkArena = &GameState->NetworkArena;
	// TODO: remove me!
	// PlaySound(
	// 	&GameState->PlayingSoundList,
	// 	WavHandle_TestMusic,
	// 	&GameState->TransientArena
	// );
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

bool AddCardToSet(
	card_game_state* SceneState,
	card_set_type CardSetType,
	player_id Owner,
	card* Card
)
{
	// NOTE: utility function for adding a card to any set, ignoring any rules 
	// CONT: checks (probably only used for network sync?)
	card_set* CardSet = NULL;
	if(CardSetType == CardSet_Hand)
	{
		CardSet = SceneState->Hands + Owner;
	}
	else if(CardSetType == CardSet_Tableau)
	{
		CardSet = SceneState->Tableaus + Owner;
	}
	else if(CardSetType == CardSet_Stack)
	{
		// NOTE: different return path for stack
		AddCardToStack(SceneState, Card);
		return true;
	}
	else
	{
		ASSERT(false);
	}

	return AddCardToSet(CardSet, Card);
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
	if(SceneState->CurrentTurn == Player)
	{
		TurnTimerCeil = Int32Ceil(SceneState->TurnTimer);
	}
	else
	{
		TurnTimerCeil = Int32Ceil(SceneState->NextTurnTimer);
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

	for(uint32_t CardIndex = 0; CardIndex < SceneState->MaxCards; CardIndex++)
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
							NormalPlayCard(GameState, SceneState, Card);
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
								GameState, SceneState, SelectedCard, Card
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
							AttackCard(
								GameState, SceneState, SelectedCard, Card
							);
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
								GameState, SceneState, SelectedCard, Card
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

	for(uint32_t CardIndex = 0; CardIndex < SceneState->MaxCards; CardIndex++)
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
						WasPlayed = StackBuildingPlayCard(
							GameState, SceneState, Card
						);
						if(!WasPlayed)
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

card* GetCardWithId(card_game_state* SceneState, uint32_t CardId)
{
	// NOTE: check if CardId exists
	card* Card = NULL;
	for(
		uint32_t CardIndex = 0; CardIndex < SceneState->MaxCards; CardIndex++
	)
	{
		card* CardToCheck = SceneState->Cards + CardIndex;
		if(!CardToCheck->Active)
		{
			continue;
		}
		if(CardToCheck->CardId == CardId)
		{
			Card = CardToCheck;
			break;
		}
	}

	return Card;
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
	// TODO: refactor and reuse in other scenes that require packet reading?
	if(SceneState->NetworkGame && !SceneState->IsLeader)
	{
		// TODO: handle packets coming in pieces
		bool StateUpdated = false;
		uint32_t BytesRead = 0;
		packet_reader_data* PacketReader = &SceneState->PacketReader;
		while(true)
		{
			read_packet_result ReadResult = ReadPacket(
				GameState,
				&SceneState->ConnectSocket,
				&SceneState->ListenSocket,
				PacketReader
			);

			if(ReadResult == ReadPacketResult_Complete)
			{
				packet_header* Header = SceneState->PacketReader.Header;
				void* Payload = SceneState->PacketReader.Payload;
				switch(Header->Type)
				{
					case(Packet_SwitchLeader):
					{
						SceneState->IsLeader = true;
						break;
					}
					case(Packet_StateUpdate):
					{
						if(Header->FrameId > SceneState->LastFrame)
						{
							StateUpdated = true;
							state_update_payload* LeaderState = (
								(state_update_payload*) Payload
							);
							if(
								SceneState->CurrentTurn != 
								GetOpponent(LeaderState->CurrentTurn)
							)
							{
								SwitchTurns(GameState, SceneState);
							}
							SceneState->TurnTimer = LeaderState->TurnTimer;
							SceneState->NextTurnTimer = (
								LeaderState->NextTurnTimer
							);

							SceneState->StackBuilding = (
								LeaderState->StackBuilding
							);
							if(SceneState->StackBuilding)
							{
								if(
									SceneState->StackTurn != 
									GetOpponent(LeaderState->StackTurn)
								)
								{
									SwitchStackTurns(GameState, SceneState);
								}
							}
							else
							{
								SceneState->StackTurn = GetOpponent(
									LeaderState->StackTurn
								);
							}

							SceneState->PlayerLife[Player_One] = (
								LeaderState->PlayerLife[Player_Two]
							);
							SceneState->PlayerLife[Player_Two] = (
								LeaderState->PlayerLife[Player_One]
							);

							SceneState->LastFrame = Header->FrameId;

							// NOTE: resources are organized relative to the 
							// CONT: leader
							SceneState->PlayerResources[Player_One] = (
								LeaderState->PlayerResources[Player_Two]
							);
							SceneState->PlayerResources[Player_Two] = (
								LeaderState->PlayerResources[Player_One]
							);
						}
						break;
					}
					case(Packet_CardUpdate):
					{
						card_update_payload* CardUpdate = (
							(card_update_payload*) Payload
						);
						uint32_t CardId = CardUpdate->CardId;
						card* CardToChange = GetCardWithId(
							SceneState, CardId
						);

						player_id CardOwner = GetOpponent(
							CardUpdate->Owner
						);
						bool IsNewCard = CardToChange == NULL;
						if(IsNewCard)
						{
							CardToChange = GetInactiveCard(
								SceneState, CardId
							);
							InitCardWithDef(
								SceneState,
								CardToChange,
								SceneState->Definitions,
								CardUpdate->DefId,
								CardOwner
							);
						}
						CardToChange->MissedUpdates = 0;

						if(
							CardToChange->LastFrame >= Header->FrameId && 
							!IsNewCard
						)
						{
							break;
						}
						else
						{
							CardToChange->LastFrame = Header->FrameId;
						}

						if(
							IsNewCard || 
							CardToChange->SetType != CardUpdate->SetType
						)
						{
							if(!IsNewCard)
							{
								RemoveCardFromSet(SceneState, CardToChange);
							}
							AddCardToSet(
								SceneState,
								CardUpdate->SetType,
								CardOwner,
								CardToChange
							);
						}

						// NOTE: player resource deltas are always relative,  
						CardToChange->PlayDelta[RelativePlayer_Self] = (
							CardUpdate->PlayDelta[RelativePlayer_Self]
						);
						CardToChange->PlayDelta[RelativePlayer_Opp] = (
							CardUpdate->PlayDelta[RelativePlayer_Opp]
						);
						CardToChange->TapDelta[RelativePlayer_Self] = (
							CardUpdate->TapDelta[RelativePlayer_Self]
						);
						CardToChange->TapDelta[RelativePlayer_Opp] = (
							CardUpdate->TapDelta[RelativePlayer_Opp]
						);
						
						CardToChange->TurnStartPlayDelta[RelativePlayer_Self] = (
							CardUpdate->TurnStartPlayDelta[RelativePlayer_Self]
						);
						CardToChange->TurnStartPlayDelta[RelativePlayer_Opp] = (
							CardUpdate->TurnStartPlayDelta[RelativePlayer_Opp]
						);

						CardToChange->TimeLeft = CardUpdate->TimeLeft;
						CardToChange->TapsAvailable = CardUpdate->TapsAvailable;
						CardToChange->TimesTapped = CardUpdate->TimesTapped;
						CardToChange->Attack = CardUpdate->Attack;
						CardToChange->TurnStartAttack= (
							CardUpdate->TurnStartAttack
						);
						CardToChange->Health = CardUpdate->Health;
						CardToChange->TurnStartHealth = (
							CardUpdate->TurnStartHealth
						);
						CardToChange->TableauTags = CardUpdate->TableauTags;
						CardToChange->StackTags = CardUpdate->StackTags;

						break;
					}
					case(Packet_RemoveCard):
					{
						remove_card_payload* RemoveCardPayload = (
							(remove_card_payload*) Payload
						);
						uint32_t CardId = RemoveCardPayload->CardId;
						card* CardToRemove = GetCardWithId(
							SceneState, CardId
						);
						SafeRemoveCard(GameState, SceneState, CardToRemove);
						break;
					}
					case(Packet_DeckUpdate):
					{
						deck_update_payload* DeckUpdatePayload = (
							(deck_update_payload*) Payload
						);
						player_id Owner = GetOpponent(
							DeckUpdatePayload->Owner
						);
						deck* Deck = SceneState->Decks + Owner;
						Deck->InDeckCount = DeckUpdatePayload->InDeckCount;
						memcpy(
							Deck->InDeck,
							(
								(uint8_t*) DeckUpdatePayload +
								DeckUpdatePayload->Offset
							),
							sizeof(uint32_t) * Deck->InDeckCount
						);
						break;
					}
					case(Packet_RandSeed):
					{
						rand_seed_payload* RandSeedPayload = (
							(rand_seed_payload*) Payload
						);
						srand(RandSeedPayload->Seed);
						break;
					}
					case(Packet_Ready):
					{
						break;
					}
					default:
					{
						// TODO: logging
						ClearSocket(&SceneState->ConnectSocket);
						break;
					}
				}
				ReadPacketEnd(PacketReader);
			}
			else if(
				ReadResult == ReadPacketResult_Incomplete ||
				ReadResult == ReadPacketResult_PeerReset
			)
			{
				break;
			}
			else if(ReadResult == ReadPacketResult_Error)
			{
				ReadPacketEnd(PacketReader);
			}
			else
			{
				ASSERT(false);
			}
		}

		for(
			uint32_t CardIndex = 0;
			CardIndex < SceneState->MaxCards;
			CardIndex++
		)
		{
			card* Card = SceneState->Cards + CardIndex;
			if(Card->Active && StateUpdated)
			{
				if(Card->MissedUpdates > MISSED_UPDATES_BEFORE_DESTRUCTION)
				{
					SafeRemoveCard(GameState, SceneState, Card);
				}
				else
				{
					Card->MissedUpdates++;
				}
			}
		}

		AlignCardSet(SceneState->Hands + Player_One);
		AlignCardSet(SceneState->Hands + Player_Two);
		AlignCardSet(SceneState->Tableaus + Player_One);
		AlignCardSet(SceneState->Tableaus + Player_Two);
	}
	// SECTION STOP: Read leader state

	// NOTE: storing frame start stack turn has to be done after updating 
	// CONT: card game state
	player_id FrameStartStackTurn = SceneState->StackTurn;

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
					uint32_t CardIndex = 0;
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

		SwitchTurns(GameState, SceneState);
		for(
			uint32_t CardIndex = 0;
			CardIndex < SceneState->MaxCards;
			CardIndex++
		)
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
			uint32_t CardIndex = 0;
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
							SafeRemoveCardAndAlign(GameState, SceneState, Card);
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
		bool SwitchingLeader = (
			EndTurn || (FrameStartStackTurn != SceneState->StackTurn)
		);
		{
			state_update_packet* StatePacket = PushStruct(
				FrameArena, state_update_packet
			);
			packet_header* Header = &StatePacket->Header;
			state_update_payload* Payload = &StatePacket->Payload;
			
			Payload->CurrentTurn = SceneState->CurrentTurn;
			Payload->TurnTimer = SceneState->TurnTimer;
			Payload->NextTurnTimer = SceneState->NextTurnTimer;
			Payload->PlayerResources[Player_One] = (
				SceneState->PlayerResources[Player_One]
			);
			Payload->PlayerResources[Player_Two] = (
				SceneState->PlayerResources[Player_Two]
			);
			Payload->StackTurn = SceneState->StackTurn;
			Payload->StackBuilding = SceneState->StackBuilding;
			Payload->PlayerLife[Player_One] = (
				SceneState->PlayerLife[Player_One]
			);
			Payload->PlayerLife[Player_Two] = (
				SceneState->PlayerLife[Player_Two]
			);

			Header->DataSize = sizeof(state_update_packet);
			InitPacketHeader(
				GameState, Header, Packet_StateUpdate, (uint8_t*) Payload
			);
			if(SwitchingLeader)
			{
				SocketSendErrorCheck(
					GameState,
					&SceneState->ConnectSocket,
					&SceneState->ListenSocket,
					Header
				);
			}
			else
			{
				ThrottledSocketSendErrorCheck(
					GameState, &SceneState->ConnectSocket, Header
				);
			}

			for(uint32_t Owner = Player_One; Owner < Player_Count; Owner++)
			{
				deck* Deck = SceneState->Decks + Owner;
				deck_update_packet* DeckUpdatePacket = PushStruct(
					FrameArena, deck_update_packet
				);
				
				packet_header* DeckUpdateHeader = &DeckUpdatePacket->Header;
				deck_update_payload* DeckUpdatePayload = (
					&DeckUpdatePacket->Payload
				);

				uint32_t* InDeckHandles = PushArray(
					FrameArena, Deck->InDeckCount, uint32_t
				);
				DeckUpdatePayload->Owner = (player_id) Owner;
				DeckUpdatePayload->InDeckCount = Deck->InDeckCount;
				DeckUpdatePayload->Offset = sizeof(deck_update_payload);
				memcpy(
					InDeckHandles,
					Deck->InDeck,
					sizeof(uint32_t) * Deck->InDeckCount
				);

				DeckUpdateHeader->DataSize = (
					sizeof(deck_update_packet) +
					Deck->InDeckCount * sizeof(uint32_t)
				);
				InitPacketHeader(
					GameState,
					DeckUpdateHeader,
					Packet_DeckUpdate,
					(uint8_t*) DeckUpdatePayload
				);

				if(SwitchingLeader)
				{
					SocketSendErrorCheck(
						GameState,
						&SceneState->ConnectSocket,
						&SceneState->ListenSocket,
						&DeckUpdatePacket->Header
					);
				}
				else
				{
					ThrottledSocketSendErrorCheck(
						GameState,
						&SceneState->ConnectSocket,
						&DeckUpdatePacket->Header
					);
				}
			}
		}

		for(
			uint32_t CardIndex = 0;
			CardIndex < SceneState->MaxCards;
			CardIndex++
		)
		{
			card* Card = SceneState->Cards + CardIndex;
			if(!Card->Active)
			{
				continue;
			}

			card_update_packet* CardPacket = PushStruct(
				FrameArena, card_update_packet
			);
			card_update_payload* Payload = &CardPacket->Payload;
			
			Payload->CardId = Card->CardId;
			Payload->DefId = Card->Definition->Id;
			Payload->Owner = Card->Owner;
			Payload->SetType = Card->SetType;

			Payload->PlayDelta[RelativePlayer_Self] = (
				Card->PlayDelta[RelativePlayer_Self]
			);
			Payload->PlayDelta[RelativePlayer_Opp] = (
				Card->PlayDelta[RelativePlayer_Opp]
			);
			Payload->TapDelta[RelativePlayer_Self] = (
				Card->TapDelta[RelativePlayer_Self]
			);
			Payload->TapDelta[RelativePlayer_Opp] = (
				Card->TapDelta[RelativePlayer_Opp]
			);
			
			Payload->TurnStartPlayDelta[RelativePlayer_Self] = (
				Card->TurnStartPlayDelta[RelativePlayer_Self]
			);
			Payload->TurnStartPlayDelta[RelativePlayer_Opp] = (
				Card->TurnStartPlayDelta[RelativePlayer_Opp]
			);

			Payload->TimeLeft = Card->TimeLeft;
			Payload->TapsAvailable = Card->TapsAvailable;
			Payload->TimesTapped = Card->TimesTapped;
			Payload->Attack = Card->Attack;
			Payload->TurnStartAttack= Card->TurnStartAttack;
			Payload->Health = Card->Health;
			Payload->TurnStartHealth = Card->TurnStartHealth;
			Payload->TableauTags = Card->TableauTags;
			Payload->StackTags = Card->StackTags;

			packet_header* Header = &CardPacket->Header;
			Header->DataSize = sizeof(card_update_packet);
			InitPacketHeader(
				GameState, Header, Packet_CardUpdate, (uint8_t*) Payload
			);
			if(SwitchingLeader)
			{
				SocketSendErrorCheck(
					GameState,
					&SceneState->ConnectSocket,
					&SceneState->ListenSocket,
					Header
				);
			}
			else
			{
				ThrottledSocketSendErrorCheck(
					GameState, &SceneState->ConnectSocket, Header
				);
			}
		}

		if(SwitchingLeader)
		{
			// NOTE: need to complete all send packet jobs because 
			// CONT: otherwise we could have the far end become leader before
			// CONT: state has synchronized
			// CONT: TCP is ordered, threads are not
			PlatformCompleteAllJobsAtPriority(
				GameState->JobQueue, JobPriority_SendPacket
			);
			// NOTE: switch leadership to follower
			switch_leader_packet* SwitchLeaderPacket = PushStruct(
				FrameArena, switch_leader_packet
			);
			packet_header* Header = &SwitchLeaderPacket->Header;
			Header->DataSize = sizeof(switch_leader_packet);
			InitPacketHeader(GameState, Header, Packet_SwitchLeader, NULL);

			SocketSendErrorCheck(
				GameState,
				&SceneState->ConnectSocket,
				&SceneState->ListenSocket,
				Header
			);
			SceneState->IsLeader = false;
		}
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
			uint32_t CardIndex = 0;
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