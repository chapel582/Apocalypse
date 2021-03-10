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
#define RESOURCE_LEFT_PADDING 150.0f

#define MAX_RESOURCE_STRING_SIZE 40

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

bool CanChangeTimers(card_game_state* SceneState, card* Card)
{
	player_id Owner = Card->Owner;
	player_id Opponent = GetOpponent(Owner);
	if((SceneState->NextTurnTimer[Owner] + Card->SelfPlayDelta) < 0)
	{
		return false;
	}
	else if((SceneState->NextTurnTimer[Opponent] + Card->OppPlayDelta) < 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void ChangeTimers(card_game_state* SceneState, card* Card)
{
	player_id Owner = Card->Owner;
	player_id Opponent = GetOpponent(Owner);
	SceneState->NextTurnTimer[Owner] += Card->SelfPlayDelta;
	SceneState->NextTurnTimer[Opponent] += Card->OppPlayDelta;
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
	Card->SelfPlayDelta = Definition->SelfPlayDelta;
	Card->TurnStartSelfPlayDelta = Definition->SelfPlayDelta;
	Card->OppPlayDelta = Definition->OppPlayDelta;
	Card->TurnStartOppPlayDelta = Definition->OppPlayDelta;
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

void InitCardWithCardData(card* Card, card_data* CardData)
{
	Card->LastFrame = 0;
	Card->CardId = CardData->CardId;
	Card->Definition = CardData->Definition;
	
	Card->Owner = CardData->Owner;
	Card->TapsAvailable = CardData->TapsAvailable;
	Card->SelfPlayDelta = CardData->SelfPlayDelta;
	Card->OppPlayDelta = CardData->OppPlayDelta;
	Card->Attack = CardData->Attack;
	Card->Health = CardData->Health;
	Card->TableauTags = CardData->TableauTags;
	Card->StackTags = CardData->StackTags;

	Card->TurnStartAttack = Card->Attack;
	Card->TurnStartHealth = Card->Health;
	Card->TurnStartSelfPlayDelta = Card->SelfPlayDelta;
	Card->TurnStartOppPlayDelta = Card->OppPlayDelta;
}

void AddCardToCardDataSet(card* Card, card_data_set* CardDataSet)
{
	ASSERT(CardDataSet->CardCount < MAX_CARDS_PER_DATA_SET);
	card_data* CardData = CardDataSet->Cards + CardDataSet->CardCount;
	CardDataSet->CardCount++;

	*CardData = {};

	CardData->CardId = Card->CardId;
	CardData->Definition = Card->Definition;
	
	CardData->Owner = Card->Owner;
	CardData->TapsAvailable = Card->TapsAvailable;
	CardData->SelfPlayDelta = Card->SelfPlayDelta;
	CardData->OppPlayDelta = Card->OppPlayDelta;
	CardData->Attack = Card->Attack;
	CardData->Health = Card->Health;
	CardData->TableauTags = Card->TableauTags;
	CardData->StackTags = Card->StackTags;
}

void RemoveCardsFromDataSet(
	card_data_set* DataSet, uint32_t StartIndex, uint32_t RemoveCount
)
{
	ASSERT(RemoveCount <= DataSet->CardCount);
	memcpy(
		DataSet->Cards + StartIndex,
		DataSet->Cards + StartIndex + RemoveCount,
		(DataSet->CardCount - RemoveCount) * sizeof(card_data)
	);
	DataSet->CardCount -= RemoveCount;
}

void ShuffleDrawSet(
	card_game_state* SceneState, player_id Player
)
{
	card_data_set* DrawSet = SceneState->DrawSets + Player;
	for(uint32_t Index = 0; Index < DrawSet->CardCount; Index++)
	{
		uint32_t NewIndex = rand() % DrawSet->CardCount;
		card_data Temp = DrawSet->Cards[NewIndex];
		card_data* OldPlace = DrawSet->Cards + Index;
		card_data* NewPlace = DrawSet->Cards + NewIndex;
		*NewPlace = *OldPlace;
		*OldPlace = Temp;
	}
}

void DiscardFullHand(
	game_state* GameState, card_game_state* SceneState, player_id Player
)
{
	card_set* Hand = SceneState->Hands + Player;
	card_data_set* DiscardSet = SceneState->DiscardSets + Player;

	for(uint32_t CardIndex = 0; CardIndex < MAX_CARDS_PER_SET; CardIndex++)
	{
		if(Hand->CardCount == 0)
		{
			break;
		}

		card* Card = Hand->Cards[CardIndex];
		if(Card == NULL)
		{
			continue;
		}

		AddCardToCardDataSet(Card, DiscardSet);
		SafeRemoveCard(GameState, SceneState, Card);
	}
	AlignCardSet(Hand);
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
	card_data_set* DrawSet = SceneState->DrawSets + Owner;
	card_data* CardData = DrawSet->Cards;
	
	card* Card = GetInactiveCard(SceneState);
	InitCardWithCardData(Card, CardData);
	AddCardAndAlign(CardSet, Card);
	RemoveCardsFromDataSet(DrawSet, 0, 1);
	return Card;
}

void DrawFullHand(card_game_state* SceneState, player_id Player)
{
	card_set* Hand = SceneState->Hands + Player;
	ASSERT(Hand->CardCount == 0);

	card_data_set* DrawSet = SceneState->DrawSets + Player;
	
	int CardsToDraw = 0;
	if(DrawSet->CardCount >= DEFAULT_HAND_SIZE)
	{
		CardsToDraw = DEFAULT_HAND_SIZE;
	}
	else
	{
		card_data_set* DiscardSet = SceneState->DiscardSets + Player;
		memcpy(DrawSet->Cards, DiscardSet->Cards, DiscardSet->CardCount);
		DrawSet->CardCount = DiscardSet->CardCount;
		DiscardSet->CardCount = 0;
		ShuffleDrawSet(SceneState, Player);

		if(DrawSet->CardCount >= DEFAULT_HAND_SIZE)
		{
			CardsToDraw = DEFAULT_HAND_SIZE;
		}
		else
		{
			CardsToDraw = DrawSet->CardCount;
		}
	}

	for(int CardIndex = 0; CardIndex < CardsToDraw; CardIndex++)
	{
		card* Card = GetInactiveCard(SceneState);
		card_data* CardData = DrawSet->Cards + CardIndex;

		InitCardWithCardData(Card, CardData);
		AddCardToSet(Hand, Card);
	}
	AlignCardSet(&SceneState->Hands[Player]);
	RemoveCardsFromDataSet(DrawSet, 0, CardsToDraw);
}

void InitDeckCard(
	card_definition** DeckCard, card_definitions* Definitions, uint32_t CardId
)
{
	// TODO: Might want a way to protect against card id out of range
	ASSERT(CardId < Definitions->NumCards);
	*DeckCard = Definitions->Array + CardId;
}

bool CheckAndTap(card_game_state* SceneState, card* Card)
{
	// NOTE: only activate card if you have the resources for it
	bool Tapped = Card->TimesTapped < Card->TapsAvailable;
	if(Tapped)
	{
		// NOTE: card effects on tap can be added here
		Card->TimesTapped++;
	}
	return Tapped;
}

bool CheckAndPlay(card_game_state* SceneState, card* Card)
{
	// NOTE: only activate card if you have the resources for it
	bool Played = CanChangeTimers(SceneState, Card);
	if(Played)
	{		
		// NOTE: card effects on play can be added here		
		ChangeTimers(SceneState, Card);
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

card_game_args* StartCardGamePrepCommon(game_state* GameState)
{
	card_game_args* SceneArgs = PushStruct(
		&GameState->SceneArgsArena, card_game_args
	);
	*SceneArgs = {};

	GameState->SceneArgs = SceneArgs;
	GameState->Scene = SceneType_CardGame; 

	return SceneArgs;
}

void StartCardGamePrep(
	game_state* GameState, char* P1DeckName, char* P2DeckName
)
{
	// NOTE: for use in setting up a local game
	card_game_args* SceneArgs = StartCardGamePrepCommon(GameState);

	char Buffer[PLATFORM_MAX_PATH];
	FormatDeckPath(Buffer, sizeof(Buffer), P1DeckName);
	SceneArgs->P1Deck = LoadDeck(Buffer);
	FormatDeckPath(Buffer, sizeof(Buffer), P2DeckName);
	SceneArgs->P2Deck = LoadDeck(Buffer);
	SceneArgs->NewCardGame = true;
	SceneArgs->SeedSet = false;
}

void StartCardGamePrep(
	game_state* GameState,
	loaded_deck P1Deck,
	loaded_deck P2Deck,
	bool IsLeader,
	platform_socket* ListenSocket,
	platform_socket* ConnectSocket
)
{
	// NOTE: for use in setting up a network game
	card_game_args* SceneArgs = StartCardGamePrepCommon(GameState);

	SceneArgs->IsLeader = IsLeader;

	SceneArgs->ListenSocket = {};
	SceneArgs->ConnectSocket = {};
	if(ListenSocket)
	{
		SceneArgs->ListenSocket = *ListenSocket;
	}
	else
	{
		SceneArgs->ListenSocket.IsValid = false; 
	}
	if(ConnectSocket)
	{
		SceneArgs->ConnectSocket = *ConnectSocket;
	}
	else
	{
		SceneArgs->ConnectSocket.IsValid = false;
	}
	SceneArgs->NetworkGame = true;

	SceneArgs->P1Deck = P1Deck;
	SceneArgs->P2Deck = P2Deck;
	SceneArgs->NewCardGame = true;
	SceneArgs->SeedSet = false;
}

void ResumeCardGamePrep(
	game_state* GameState,
	bool IsLeader,
	platform_socket* ListenSocket,
	platform_socket* ConnectSocket,
	uint32_t Seed,
	bool SeedSet
)
{
	// NOTE: for use in setting up a network game
	card_game_args* SceneArgs = StartCardGamePrepCommon(GameState);

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
	SceneArgs->NewCardGame = false;
	SceneArgs->SeedSet = true;
	SceneArgs->Seed = Seed;
}

void StartCardGameDataSetup(
	game_state* GameState,
	card_game_state* SceneState,
	card_game_args* SceneArgs,
	uint32_t WindowWidth,
	uint32_t WindowHeight
)
{
	/* NOTE:
	for setting up card_game_state data (anything not related to 
	actual game state. ignores stuff like setting life, resources, drawing 
	cards)
	*/
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

	SceneState->Definitions = DefineCards(&GameState->TransientArena);
	SceneState->Decks = PushArray(
		&GameState->TransientArena, Player_Count, deck
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

	SceneState->Alert = MakeAlert();

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
	// TODO: handle ui scaling here
	float Padding = 95.0f;
	vector2 PlayerLifeRectDim = Vector2(145.0f, 30.0f);
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

	float YPadding = 10.0f;
	rectangle* DrawRects = SceneState->DrawRects;
	DrawRects[Player_One] = MakeRectangle(
		(
			GetBottomLeft(SceneState->PlayerLifeRects[Player_One]) -
			Vector2(0.0f, PlayerLifeRectDim.Y + YPadding)
		),
		PlayerLifeRectDim
	);
	DrawRects[Player_Two] = MakeRectangle(
		(
			GetTopLeft(SceneState->PlayerLifeRects[Player_Two]) +
			Vector2(0.0f, YPadding)
		),
		PlayerLifeRectDim
	);

	rectangle* DiscardRects = SceneState->DiscardRects;
	DiscardRects[Player_One] = MakeRectangle(
		(
			GetBottomLeft(DrawRects[Player_One]) -
			Vector2(0.0f, PlayerLifeRectDim.Y + YPadding)
		),
		PlayerLifeRectDim
	);
	DiscardRects[Player_Two] = MakeRectangle(
		GetTopLeft(DrawRects[Player_Two]) + Vector2(0.0f, YPadding),
		PlayerLifeRectDim
	);

	SceneState->PacketReader = {};
	SceneState->PacketReader.NetworkArena = &GameState->NetworkArena;

	if(SceneArgs->SeedSet)
	{
		SceneState->Seed = SceneArgs->Seed;
	}
}

void StartCardGame(
	game_state* GameState, uint32_t WindowWidth, uint32_t WindowHeight
)
{
	card_game_args* SceneArgs = (card_game_args*) (
		GameState->SceneArgs
	);

	GameState->SceneState = PushStruct(
		&GameState->TransientArena, card_game_state
	);

	card_game_state* SceneState = (card_game_state*) GameState->SceneState;

	if(SceneArgs->NewCardGame)
	{
		StartCardGameDataSetup(
			GameState, SceneState, SceneArgs, WindowWidth, WindowHeight
		);

		loaded_deck P1Deck = SceneArgs->P1Deck;
		loaded_deck P2Deck = SceneArgs->P2Deck;

		SceneState->NextCardId = 0;
		if(SceneState->NetworkGame && !SceneState->IsLeader)
		{
			SceneState->NextCardId = 1 << 16;
		}

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

			card_definition** DeckCard = Deck->Cards;
			*DeckCard = NULL;
			InitDeckCard(
				DeckCard, SceneState->Definitions, LoadedDeck->Ids[0]
			);
			for(
				int CardIndex = 1;
				CardIndex < LoadedDeck->Header.CardCount;
				CardIndex++
			)
			{
				DeckCard = Deck->Cards + CardIndex;
				*DeckCard = NULL;
				InitDeckCard(
					DeckCard,
					SceneState->Definitions,
					LoadedDeck->Ids[CardIndex]
				);
			}
		}

		if(!SceneArgs->SeedSet)
		{
			// TODO: this may not be a great way to seed. might be exploitable?
			SceneState->Seed = (uint32_t) time(NULL);
		}
		uint32_t Seed = SceneState->Seed;
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

		// NOTE: initialize discard sets
		card_data_set* DiscardSet = SceneState->DiscardSets + Player_One;
		*DiscardSet = {};
		DiscardSet->Type = CardDataSet_Discard;
		DiscardSet = SceneState->DiscardSets + Player_Two;
		*DiscardSet = {};
		DiscardSet->Type = CardDataSet_Discard;

		if(
			!SceneState->NetworkGame || 
			(SceneState->NetworkGame && SceneState->IsLeader)
		)
		{
			// NOTE: initialize draw sets
			for(
				uint32_t PlayerIndex = Player_One;
				PlayerIndex < Player_Count;
				PlayerIndex++
			)
			{
				deck* Deck = SceneState->Decks + PlayerIndex;
				card_data_set* DrawSet = SceneState->DrawSets + PlayerIndex;
				*DrawSet = {};
				DrawSet->Type = CardDataSet_Draw;
				for(
					uint32_t CardIndex = 0;
					CardIndex < Deck->CardCount;
					CardIndex++
				)
				{
					card_definition* CardDefinition = Deck->Cards[CardIndex];
					card_data* CardData = DrawSet->Cards + CardIndex;
					
					CardData->CardId = SceneState->NextCardId++;
					CardData->Definition = CardDefinition;
					
					CardData->Owner = (player_id) PlayerIndex;
					CardData->TapsAvailable = CardDefinition->TapsAvailable;
					CardData->SelfPlayDelta = CardDefinition->SelfPlayDelta;
					CardData->OppPlayDelta = CardDefinition->OppPlayDelta;
					CardData->Attack = CardDefinition->Attack;
					CardData->Health = CardDefinition->Health;
					CardData->TableauTags = CardDefinition->TableauTags;
					CardData->StackTags = CardDefinition->StackTags;

					DrawSet->CardCount++;
				}
			}
			ShuffleDrawSet(SceneState, Player_One);
			ShuffleDrawSet(SceneState, Player_Two);

			DrawFullHand(SceneState, Player_One);
			DrawFullHand(SceneState, Player_Two);
		}

		SetTurnTimer(SceneState, DEFAULT_NEXT_TURN_TIMER);
		SceneState->NextTurnTimer[Player_One] = DEFAULT_NEXT_TURN_TIMER;
		SceneState->NextTurnTimer[Player_Two] = DEFAULT_NEXT_TURN_TIMER;

		SceneState->PlayerLife[Player_One] = 100.0f;
		SceneState->PlayerLife[Player_Two] = 100.0f;
		if(SceneState->IsLeader)
		{
			SceneState->SyncState = SyncState_Send;
		}
		else
		{
			SceneState->SyncState = SyncState_Read;
		}
	}
	else
	{
		StartCardGameDataSetup(
			GameState, SceneState, SceneArgs, WindowWidth, WindowHeight
		);
		SceneState->SyncState = SyncState_Read;
		// TODO: handle unique ids
	}
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

void PushNextTurnTimer(
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
	TurnTimerCeil = Int32Ceil(SceneState->NextTurnTimer[Player]);
	
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
						bool WasPlayed = CheckAndPlay(SceneState, Card);
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
						bool Tapped = CheckAndTap(SceneState, SelectedCard);
						if(Tapped)
						{
							DeselectCard(SceneState);
							attack_card_result Result = AttackCard(
								GameState, SceneState, SelectedCard, Card
							);
							// NOTE: can add effects that happen after an attack
							// CONT: here 
						}
					}
				}
				else if(Card->SetType == CardSet_Tableau)
				{
					if(SelectedCard == NULL)
					{
						if(Card->TimesTapped < Card->TapsAvailable)
						{
							bool CanActivate = CanChangeTimers(
								SceneState, Card
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
						if(SelectedCard != Card)
						{
							DeselectCard(SceneState);
							SelectCard(SceneState, Card);
						}
						else
						{
							DeselectCard(SceneState);
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
						bool Tapped = CheckAndTap(SceneState, SelectedCard);
						if(Tapped)
						{
							DeselectCard(SceneState);
							AttackCard(
								GameState, SceneState, SelectedCard, Card
							);
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
				bool Tapped = CheckAndTap(SceneState, SelectedCard);
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
		else if(
			PointInRectangle(
				MouseEventWorldPos, SceneState->DrawRects[Player]
			)
		)
		{
			SceneState->ViewingCardDataSet = SceneState->DrawSets + Player;
		}
		else if(
			PointInRectangle(
				MouseEventWorldPos, SceneState->DiscardRects[Player]
			)
		)
		{
			SceneState->ViewingCardDataSet = SceneState->DiscardSets + Player;
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
					bool WasPlayed = CheckAndPlay(SceneState, Card);
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

inline void CardDataSetViewPrimaryUpHandler(
	game_state* GameState,
	card_game_state* SceneState,
	vector2 MouseEventWorldPos
)
{
	card_data_set* DataSet = SceneState->ViewingCardDataSet;
	ASSERT(DataSet != NULL);

	bool AnyClicked = false;
	for(uint32_t CardIndex = 0; CardIndex < DataSet->CardCount; CardIndex++)
	{
		card_data* CardData = DataSet->Cards + CardIndex;
		rectangle Rectangle = CardData->Rectangle;
		if(PointInRectangle(MouseEventWorldPos, Rectangle))
		{
			DataSet->ShowInfoCard = CardData;
			AnyClicked = true;
			break;
		}
	}

	if(!AnyClicked)
	{
		DataSet->ShowInfoCard = NULL;
	}
}

bool StandardKeyboardHandler(
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

bool StackBuildingKeyboardHandler(
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

void CardDataSetViewKeyboardHandler(
	game_state* GameState,
	card_game_state* SceneState,
	game_keyboard_event* KeyboardEvent
)
{
	if(
		!KeyboardEvent->IsDown && 
		(KeyboardEvent->IsDown != KeyboardEvent->WasDown)
	)
	{
		switch(KeyboardEvent->Code)
		{			
			case(0x1B): // NOTE: Escape V-code
			{
				card_data_set* DataSet = SceneState->ViewingCardDataSet;
				DataSet->ShowInfoCard = NULL;

				SceneState->ViewingCardDataSet = NULL;
				break;
			}
		}
	}
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

void SendCardDataSetUpdates(
	game_state* GameState,
	card_game_state* SceneState,
	memory_arena* FrameArena,
	card_data_set* DataSet,
	player_id Owner,
	bool SwitchingLeader
)
{
	card_data_set_update_packet* UpdatePacket = PushStruct(
		FrameArena, card_data_set_update_packet
	);
	card_data_update* CardUpdates = PushArray(
		FrameArena, DataSet->CardCount, card_data_update, 1
	);
	
	packet_header* UpdateHeader = &UpdatePacket->Header;
	card_data_set_update_payload* UpdatePayload = &UpdatePacket->Payload;

	UpdatePayload->Owner = Owner;
	UpdatePayload->Type = DataSet->Type;
	UpdatePayload->CardCount = DataSet->CardCount;

	for(uint32_t CardIndex = 0; CardIndex < DataSet->CardCount; CardIndex++)
	{
		card_data* CardData = DataSet->Cards + CardIndex;
		card_data_update* Update = CardUpdates + CardIndex;

		Update->CardId = CardData->CardId;
		Update->DefId = CardData->Definition->Id;
		
		Update->TapsAvailable = CardData->TapsAvailable;
		Update->SelfPlayDelta = CardData->SelfPlayDelta;
		Update->OppPlayDelta = CardData->OppPlayDelta;
		Update->Attack = CardData->Attack;
		Update->Health = CardData->Health;
		Update->TableauTags = CardData->TableauTags;
		Update->StackTags = CardData->StackTags;
	}

	UpdateHeader->DataSize = (
		sizeof(card_data_set_update_packet) +
		UpdatePayload->CardCount * sizeof(card_data_update)
	);
	InitPacketHeader(
		GameState,
		UpdateHeader,
		Packet_CardDataSetUpdate,
		(uint8_t*) UpdatePayload
	);

	if(SwitchingLeader)
	{
		SocketSendErrorCheck(
			GameState,
			&SceneState->ConnectSocket,
			&SceneState->ListenSocket,
			&UpdatePacket->Header
		);
	}
	else
	{
		ThrottledSocketSendErrorCheck(
			GameState,
			&SceneState->ConnectSocket,
			&SceneState->ListenSocket,
			&UpdatePacket->Header
		);
	}
}

void SendGameState(
	game_state* GameState,
	card_game_state* SceneState,
	bool SwitchingLeader,
	bool AlwaysSend
)
{
	memory_arena* FrameArena = &GameState->FrameArena;
	{
		state_update_packet* StatePacket = PushStruct(
			FrameArena, state_update_packet
		);
		packet_header* Header = &StatePacket->Header;
		state_update_payload* Payload = &StatePacket->Payload;
		
		Payload->CurrentTurn = SceneState->CurrentTurn;
		Payload->TurnTimer = SceneState->TurnTimer;
		Payload->NextTurnTimer[Player_One] = (
			SceneState->NextTurnTimer[Player_One]
		);
		Payload->NextTurnTimer[Player_Two] = (
			SceneState->NextTurnTimer[Player_Two]
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
		if(SwitchingLeader || AlwaysSend)
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
				GameState,
				&SceneState->ConnectSocket,
				&SceneState->ListenSocket,
				Header
			);
		}

		for(uint32_t Owner = Player_One; Owner < Player_Count; Owner++)
		{
			card_data_set* DataSet = SceneState->DrawSets + Owner;
			SendCardDataSetUpdates(
				GameState,
				SceneState,
				FrameArena,
				DataSet,
				(player_id) Owner,
				SwitchingLeader
			);
			DataSet = SceneState->DiscardSets + Owner;
			SendCardDataSetUpdates(
				GameState,
				SceneState,
				FrameArena,
				DataSet,
				(player_id) Owner,
				SwitchingLeader
			);
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

		Payload->TapsAvailable = Card->TapsAvailable;
		Payload->TimesTapped = Card->TimesTapped;
		Payload->Attack = Card->Attack;
		Payload->TurnStartAttack= Card->TurnStartAttack;
		Payload->Health = Card->Health;
		Payload->TurnStartHealth = Card->TurnStartHealth;
		Payload->SelfPlayDelta = Card->SelfPlayDelta;
		Payload->TurnStartSelfPlayDelta = Card->TurnStartSelfPlayDelta;
		Payload->OppPlayDelta = Card->OppPlayDelta;
		Payload->TurnStartOppPlayDelta = Card->TurnStartOppPlayDelta;
		Payload->TableauTags = Card->TableauTags;
		Payload->StackTags = Card->StackTags;

		packet_header* Header = &CardPacket->Header;
		Header->DataSize = sizeof(card_update_packet);
		InitPacketHeader(
			GameState, Header, Packet_CardUpdate, (uint8_t*) Payload
		);
		if(SwitchingLeader || AlwaysSend)
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
				GameState,
				&SceneState->ConnectSocket,
				&SceneState->ListenSocket,
				Header
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

void CardGameLogic(
	game_state* GameState,
	card_game_state* SceneState,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame,
	bool* EndTurn
)
{
	ui_context* UiContext = &SceneState->UiContext;

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
				if(SceneState->ViewingCardDataSet != NULL)
				{
					CardDataSetViewPrimaryUpHandler(
						GameState, SceneState, MouseEventWorldPos
					);
				}
				else if(SceneState->StackBuilding)
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
			if(SceneState->ViewingCardDataSet != NULL)
			{
				CardDataSetViewKeyboardHandler(
					GameState, SceneState, KeyboardEvent
				);
			}
			else if(!SceneState->StackBuilding)
			{
				TempEndTurn = StandardKeyboardHandler(
					GameState, SceneState, KeyboardEvent
				);
				// NOTE: this is not unnecessary. Don't want to overwrite EndTurn 
				// CONT: with false by accident
				if(TempEndTurn)
				{
					*EndTurn = TempEndTurn;
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
	if(!(*EndTurn))
	{
		if(SceneState->StackBuilding)
		{
			if(SceneState->StackTurn == SceneState->CurrentTurn)
			{
				SceneState->TurnTimer -= DtForFrame;
			}
			else
			{
				player_id NextTurnPlayer = GetOpponent(SceneState->CurrentTurn);
				SceneState->NextTurnTimer[NextTurnPlayer] -= DtForFrame;
				if(SceneState->NextTurnTimer[NextTurnPlayer] <= 0.0f)
				{
					SceneState->NextTurnTimer[NextTurnPlayer] = 0.0f;
				}

				if(SceneState->NextTurnTimer[NextTurnPlayer] <= 0.0f)
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
			*EndTurn = true;
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
	if(*EndTurn && (!SceneState->NetworkGame || SceneState->IsLeader))
	{
		EndStackBuilding(SceneState);

		player_id NextTurnPlayer = GetOpponent(SceneState->CurrentTurn);
		SetTurnTimer(SceneState, SceneState->NextTurnTimer[NextTurnPlayer]);

		SceneState->NextTurnTimer[NextTurnPlayer] = DEFAULT_NEXT_TURN_TIMER;

		DiscardFullHand(GameState, SceneState, SceneState->CurrentTurn);
		DrawFullHand(SceneState, SceneState->CurrentTurn);

		SwitchTurns(GameState, SceneState);
		// NOTE: reset values to turn start values here (when appropriate)
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
						Card->SelfPlayDelta = Card->TurnStartSelfPlayDelta;
					}
				}
				if(HasTag(EffectTags, TableauEffect_GiveIncrease))
				{
					if(Card->SetType == CardSet_Hand)
					{
						Card->OppPlayDelta = Card->TurnStartOppPlayDelta;
					}
				}
			}
		}
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
						if(Card->Attack < 0)
						{
							Card->Attack = 0;
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
							Card->SelfPlayDelta -= 1;
						}
					}
				}
				if(HasTag(TableauTags, TableauEffect_GiveIncrease))
				{
					if(Card->SetType == CardSet_Hand)
					{
						if(WholeSecondPassed)
						{
							Card->SelfPlayDelta += 1;
						}
					}
				}
			}
			Card++;
		}
	}
	// SECTION STOP: Card update
	// SECTION STOP: Updating game state
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
	if(
		SceneState->NetworkGame &&
		(
			!SceneState->IsLeader ||
			SceneState->SyncState == SyncState_Read
		)
	)
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
					case(Packet_SetLeader):
					{
						set_leader_payload* LeaderPayload = (
							(set_leader_payload*) Payload
						);
						SceneState->IsLeader = LeaderPayload->IsLeader;
						break;
					}
					case(Packet_SetFrameCount):
					{
						set_frame_count_payload* SetFrameCountPayload = (
							(set_frame_count_payload*) Payload
						);
						GameState->FrameCount = (
							SetFrameCountPayload->FrameCount
						);
						break;
					}
					case(Packet_SyncDone):
					{
						SceneState->SyncState = SyncState_Complete;
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
							SetTurnTimer(SceneState, LeaderState->TurnTimer);
							SceneState->NextTurnTimer[Player_Two] = (
								LeaderState->NextTurnTimer[Player_One]
							);
							SceneState->NextTurnTimer[Player_One] = (
								LeaderState->NextTurnTimer[Player_Two]
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
						CardToChange->SelfPlayDelta = CardUpdate->SelfPlayDelta;
						CardToChange->TurnStartSelfPlayDelta = (
							CardUpdate->TurnStartSelfPlayDelta
						);
						CardToChange->OppPlayDelta = CardUpdate->OppPlayDelta;
						CardToChange->TurnStartOppPlayDelta = (
							CardUpdate->TurnStartOppPlayDelta
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
					case(Packet_CardDataSetUpdate):
					{
						card_data_set_update_payload* UpdatePayload = (
							(card_data_set_update_payload*) Payload
						);
						uint8_t* CardsPtr = (
							(uint8_t*)(UpdatePayload) + sizeof(*UpdatePayload)
						);
						card_data_update* CardUpdates = (card_data_update*)(
							CardsPtr
						);

						card_data_set* DataSetArray = NULL;
						if(UpdatePayload->Type == CardDataSet_Draw)
						{
							DataSetArray = SceneState->DrawSets;
						}
						else if(UpdatePayload->Type == CardDataSet_Discard)
						{
							DataSetArray = SceneState->DiscardSets;
						}
						else
						{
							ASSERT(false);
						}
						card_data_set* DataSet = (
							DataSetArray + GetOpponent(UpdatePayload->Owner)
						);
						DataSet->CardCount = UpdatePayload->CardCount;

						ASSERT(DataSet->CardCount <= MAX_CARDS_PER_DATA_SET);
						for(
							uint32_t CardIndex = 0;
							CardIndex < DataSet->CardCount;
							CardIndex++
						)
						{
							card_data* CardData = DataSet->Cards + CardIndex;
							card_data_update* Update = CardUpdates + CardIndex;

							CardData->CardId = Update->CardId;
							CardData->Definition = (
								SceneState->Definitions->Array + Update->DefId
							);
							
							CardData->TapsAvailable = Update->TapsAvailable;
							CardData->SelfPlayDelta = Update->SelfPlayDelta;
							CardData->OppPlayDelta = Update->OppPlayDelta;
							CardData->Attack = Update->Attack;
							CardData->Health = Update->Health;
							CardData->TableauTags = Update->TableauTags;
							CardData->StackTags = Update->StackTags;
						}
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

	// SECTION START: calculate data set rectangles
	if(SceneState->ViewingCardDataSet != NULL)
	{
		card_data_set* DataSet = SceneState->ViewingCardDataSet;
		uint32_t CardsPerRow = 10;

		float Width = SceneState->CardWidth;
		float ScreenWidthInWorld = ScreenDimInWorld.X;
		float SpaceSize = (
			(ScreenWidthInWorld - (CardsPerRow * Width)) / (CardsPerRow + 1)
		);
		float DistanceBetweenCardPos = SpaceSize + Width;

		vector2 Dim = Vector2(SceneState->CardWidth, SceneState->CardHeight);
		vector2 RowStart = Vector2(
			SpaceSize, ScreenDimInWorld.Y - 1.1f * Dim.Y
		);
		vector2 NextCol = Vector2(DistanceBetweenCardPos, 0.0f);
		vector2 NextRow = Vector2(0.0f, 1.1f * Dim.Y);
		uint32_t Rows = (DataSet->CardCount / CardsPerRow) + 1;
		for(uint32_t Row = 0; Row < Rows; Row++)
		{
			vector2 CurrentMin = RowStart;
			for(uint32_t Col = 0; Col < CardsPerRow; Col++)
			{
				uint32_t CardIndex = Row * CardsPerRow + Col;
				if(CardIndex >= DataSet->CardCount)
				{
					break;
				}
				card_data* CardData = DataSet->Cards + CardIndex;
				CardData->Rectangle = MakeRectangle(CurrentMin, Dim);

				CurrentMin += NextCol;
			}

			RowStart -= NextRow;
		}
	}
	// SECTION STOP: calculate data set rectangles

	if(
		(SceneState->SyncState == SyncState_Complete) ||
		!SceneState->NetworkGame
	)
	{
		// NOTE: storing frame start stack turn has to be done after updating 
		// CONT: card game state
		player_id FrameStartStackTurn = SceneState->StackTurn;

		CardGameLogic(
			GameState,
			SceneState,
			MouseEvents,
			KeyboardEvents,
			DtForFrame,
			&EndTurn
		);
		if(SceneState->NetworkGame && SceneState->IsLeader)
		{
			bool SwitchingLeader = (
				EndTurn || (FrameStartStackTurn != SceneState->StackTurn)
			);
			SendGameState(GameState, SceneState, SwitchingLeader, false);
		}
	}
	else if(SceneState->SyncState == SyncState_Send)
	{
		SendGameState(GameState, SceneState, false, true);
		
		{
			set_frame_count_packet* SetFrameCountPacket = PushStruct(
				FrameArena, set_frame_count_packet
			);
			packet_header* Header = &SetFrameCountPacket->Header;
			set_frame_count_payload* Payload = &SetFrameCountPacket->Payload;
			Payload->FrameCount = GameState->FrameCount;

			Header->DataSize = sizeof(set_frame_count_packet);
			InitPacketHeader(
				GameState, Header, Packet_SetFrameCount, (uint8_t*) Payload
			);

			SocketSendErrorCheck(
				GameState,
				&SceneState->ConnectSocket,
				&SceneState->ListenSocket,
				Header
			);
		}
		{
			set_leader_packet* SetLeaderPacket = PushStruct(
				FrameArena, set_leader_packet
			);
			packet_header* Header = &SetLeaderPacket->Header;
			set_leader_payload* Payload = &SetLeaderPacket->Payload;
			Payload->IsLeader = !SceneState->IsLeader;

			Header->DataSize = sizeof(set_leader_packet);
			InitPacketHeader(
				GameState, Header, Packet_SetLeader, (uint8_t*) Payload
			);

			SocketSendErrorCheck(
				GameState,
				&SceneState->ConnectSocket,
				&SceneState->ListenSocket,
				Header
			);
		}
		{
			sync_done_packet* SyncDonePacket = PushStruct(
				FrameArena, sync_done_packet
			);
			packet_header* Header = &SyncDonePacket->Header;

			Header->DataSize = sizeof(sync_done_packet);
			InitPacketHeader(
				GameState, Header, Packet_SyncDone, NULL
			);

			SocketSendErrorCheck(
				GameState,
				&SceneState->ConnectSocket,
				&SceneState->ListenSocket,
				Header
			);
		}

		SceneState->SyncState = SyncState_Complete;
	}
	else
	{
		ASSERT(SceneState->SyncState == SyncState_Read);
	}

	// SECTION START: Push render entries
	render_group* RenderGroup = &GameState->RenderGroup;
	PushClear(RenderGroup, Vector4(0.25f, 0.25f, 0.25f, 1.0f));
	if(SceneState->ViewingCardDataSet != NULL)
	{
		RenderGroup->ColorMultiply = Vector4(0.5f, 0.5f, 0.5f, 0.5f);
	}
	else
	{
		RenderGroup->ColorMultiply = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	PushNextTurnTimer(SceneState, Player_One, RenderGroup, Assets, FrameArena);
	PushNextTurnTimer(SceneState, Player_Two, RenderGroup, Assets, FrameArena);

	// NOTE: push current turn timer
	{
		// TODO: it'd be cool if there was a stack effect here for the "stack" turn timers
		char* TimeString = PushArray(
			FrameArena, MAX_RESOURCE_STRING_SIZE, char
		);
		snprintf(
			TimeString,
			MAX_RESOURCE_STRING_SIZE,
			"%d",
			Int32Ceil(SceneState->TurnTimer)
		);
		PushText(
			RenderGroup,
			Assets,
			FontHandle_TestFont,
			TimeString,
			MAX_RESOURCE_STRING_SIZE,
			50.0f,
			Vector2(
				ScreenDimInWorld.X - RESOURCE_LEFT_PADDING,
				(ScreenDimInWorld.Y / 2.0f)
			),
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			FrameArena
		);
	}

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
						Card->SelfPlayDelta,
						Card->OppPlayDelta,
						Card->Definition->Name,
						Card->TapsAvailable - Card->TimesTapped
					);
				}
			}
			Card++;
		}
	}
	// SECTION STOP: Push cards

	// SECTION START: Push player life totals
	{
		char* PlayerLifeString = PushArray(
			FrameArena, MAX_RESOURCE_STRING_SIZE, char
		);
		snprintf(
			PlayerLifeString,
			MAX_RESOURCE_STRING_SIZE,
			"P1 Life:%d",
			(int) SceneState->PlayerLife[Player_One]
		);

		vector4 White = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		vector4 Black = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
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
			"P2 Life:%d",
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

	// SECTION START: Push player draw/discard totals
	{
		vector4 White = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		vector4 Black = Vector4(0.0f, 0.0f, 0.0f, 1.0f);

		char* PlayerString = PushArray(
			FrameArena, MAX_RESOURCE_STRING_SIZE, char
		);
		snprintf(
			PlayerString,
			MAX_RESOURCE_STRING_SIZE,
			"P1 Draw:%d",
			(int) SceneState->DrawSets[Player_One].CardCount
		);

		rectangle* DrawRects = SceneState->DrawRects;
		rectangle* CurrentRect = DrawRects + Player_One;
		PushSizedBitmap(
			RenderGroup,
			Assets,
			BitmapHandle_TestCard2,
			GetCenter(*CurrentRect),
			Vector2(CurrentRect->Dim.X, 0.0f),
			Vector2(0.0f, CurrentRect->Dim.Y),
			White,
			1
		);
		PushText(
			RenderGroup,
			Assets,
			FontHandle_TestFont,
			PlayerString,
			MAX_RESOURCE_STRING_SIZE,
			CurrentRect->Dim.Y,
			CurrentRect->Min,
			Black,
			FrameArena,
			2
		);

		snprintf(
			PlayerString,
			MAX_RESOURCE_STRING_SIZE,
			"P2 Draw:%d",
			(int) SceneState->DrawSets[Player_Two].CardCount
		);

		CurrentRect = DrawRects + Player_Two;
		PushSizedBitmap(
			RenderGroup,
			Assets,
			BitmapHandle_TestCard2,
			GetCenter(*CurrentRect),
			Vector2(CurrentRect->Dim.X, 0.0f),
			Vector2(0.0f, CurrentRect->Dim.Y),
			White,
			1
		);
		PushText(
			RenderGroup,
			Assets,
			FontHandle_TestFont,
			PlayerString,
			MAX_RESOURCE_STRING_SIZE,
			CurrentRect->Dim.Y,
			CurrentRect->Min,
			Black,
			FrameArena,
			2
		);

		snprintf(
			PlayerString,
			MAX_RESOURCE_STRING_SIZE,
			"P1 Disc:%d",
			(int) SceneState->DiscardSets[Player_One].CardCount
		);
		rectangle* DiscardRects = SceneState->DiscardRects;
		CurrentRect = DiscardRects + Player_One;
		PushSizedBitmap(
			RenderGroup,
			Assets,
			BitmapHandle_TestCard2,
			GetCenter(*CurrentRect),
			Vector2(CurrentRect->Dim.X, 0.0f),
			Vector2(0.0f, CurrentRect->Dim.Y),
			White,
			1
		);
		PushText(
			RenderGroup,
			Assets,
			FontHandle_TestFont,
			PlayerString,
			MAX_RESOURCE_STRING_SIZE,
			CurrentRect->Dim.Y,
			CurrentRect->Min,
			Black,
			FrameArena,
			2
		);

		snprintf(
			PlayerString,
			MAX_RESOURCE_STRING_SIZE,
			"P2 Disc:%d",
			(int) SceneState->DiscardSets[Player_Two].CardCount
		);
		CurrentRect = DiscardRects + Player_Two;
		PushSizedBitmap(
			RenderGroup,
			Assets,
			BitmapHandle_TestCard2,
			GetCenter(*CurrentRect),
			Vector2(CurrentRect->Dim.X, 0.0f),
			Vector2(0.0f, CurrentRect->Dim.Y),
			White,
			1
		);
		PushText(
			RenderGroup,
			Assets,
			FontHandle_TestFont,
			PlayerString,
			MAX_RESOURCE_STRING_SIZE,
			CurrentRect->Dim.Y,
			CurrentRect->Min,
			Black,
			FrameArena,
			2
		);
	}
	// SECTION STOP: Push player draw/discard totals

	// SECTION START: push cards in card data set
	if(SceneState->ViewingCardDataSet != NULL)
	{
		uint32_t CardDataLayer = 3;
		uint32_t InfoLayer = CardDataLayer + 1;
		vector4 White = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		RenderGroup->ColorMultiply = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

		card_data_set* DataSet = SceneState->ViewingCardDataSet;
		for(uint32_t CardIndex = 0; CardIndex < DataSet->CardCount; CardIndex++)
		{
			card_data* CardData = DataSet->Cards + CardIndex;
			card_definition* Definition = CardData->Definition;

			rectangle Rectangle = CardData->Rectangle;

			// TODO: don't hardcode the layers
			vector2 Center = GetCenter(Rectangle);
			PushSizedBitmap(
				RenderGroup,
				Assets,
				BitmapHandle_TestCard2,
				Center,
				Vector2(Rectangle.Dim.X, 0.0f),
				Vector2(0.0f, Rectangle.Dim.Y),
				White,
				CardDataLayer
			);
			PushTextCentered(
				RenderGroup,
				Assets,
				FontHandle_TestFont,
				Definition->Name,
				CARD_NAME_SIZE,
				0.2f * Rectangle.Dim.Y,
				Center,
				Vector4(0, 0, 0, 1),
				FrameArena,
				CardDataLayer
			);
		}
		if(DataSet->ShowInfoCard)
		{
			card_data* CardData = DataSet->ShowInfoCard;
			card_definition* Definition = CardData->Definition;
			
			PushInfoCard(
				RenderGroup,
				Assets,
				SceneState->InfoCardCenter,
				SceneState->InfoCardXBound,
				SceneState->InfoCardYBound,
				White,
				FrameArena,
				Definition->Name,
				CardData->Attack,
				CardData->Health,
				CardData->SelfPlayDelta,
				CardData->OppPlayDelta,
				Definition->Name,
				CardData->TapsAvailable,
				InfoLayer
			);
		}
	}
	// SECTION STOP: push cards in card data set

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