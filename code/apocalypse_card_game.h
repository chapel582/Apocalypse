#ifndef APOCALYPSE_CARD_GAME_H

#define MAX_CARDS_PER_SET 7
#define RESOURCE_TO_TIME 5.0f
#define TURN_TIMER_INCREASE 10.0f

#include "apocalypse_platform.h"
#include "apocalypse_vector.h"
#include "apocalypse_rectangle.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_bitmap.h"
#include "apocalypse_render_group.h"
#include "apocalypse_assets.h"
#include "apocalypse_audio.h"
#include "apocalypse_particles.h"
#include "apocalypse_card_definitions.h"
#include "apocalypse_player_id.h"
#include "apocalypse_deck_storage.h"
#include "apocalypse_alert.h"

typedef enum
{
	CardSet_Hand,
	CardSet_Tableau,
	CardSet_Count
} card_set_type;

struct deck_card;
struct deck_card
{
	card_definition* Definition;
	deck_card* Next;
	deck_card* Previous;
};

struct deck
{
	deck_card Cards[MAX_CARDS_IN_DECK];
	// NOTE: these linked lists are doubly linked and circular 
	deck_card* InDeck;
	int InDeckLength;
	deck_card* OutOfDeck;
	int OutOfDeckLength;
};

deck_card* GetDeckCard(
	deck_card* DeckCardListHead, int DeckCardListLen, int Index
)
{
	ASSERT(Index < DeckCardListLen);
	deck_card* DeckCard = DeckCardListHead;
	for(int Pass = 0; Pass < Index; Pass++)
	{
		DeckCard = DeckCard->Next;
	}
	return DeckCard;
}

void InOutSwap(
	deck* Deck,
	deck_card* DeckCard,
	deck_card* First,
	int* ToIncrement,
	int* ToDecrement
)
{
	deck_card* OldPrevious = DeckCard->Previous;
	deck_card* OldNext = DeckCard->Next;

	if(OldPrevious != NULL)
	{
		OldPrevious->Next = OldNext;
	}
	if(OldNext != NULL)
	{
		OldNext->Previous = OldPrevious;
	}
	(*ToDecrement)--;

	if(First != NULL)
	{
		First->Previous = DeckCard;
	}

	DeckCard->Next = First;
	DeckCard->Previous = NULL;
	(*ToIncrement)++;
}

void InDeckToOutDeck(deck* Deck, deck_card* DeckCard)
{
	if(Deck->InDeck == DeckCard)
	{
		Deck->InDeck = DeckCard->Next;
	}
	InOutSwap(
		Deck,
		DeckCard,
		Deck->OutOfDeck,
		&Deck->OutOfDeckLength,
		&Deck->InDeckLength
	);
	Deck->OutOfDeck = DeckCard;
}

void OutDeckToInDeck(deck* Deck, deck_card* DeckCard)
{
	if(Deck->OutOfDeck == DeckCard)
	{
		Deck->OutOfDeck = DeckCard->Next;
	}
	InOutSwap(
		Deck,
		DeckCard,
		Deck->InDeck,
		&Deck->InDeckLength,
		&Deck->OutOfDeckLength
	);
	Deck->InDeck = DeckCard;
}

struct card
{
	card_definition* Definition;
	player_resources PlayDelta[Player_Count];
	player_resources TapDelta[Player_Count];
	
	player_resources TurnStartPlayDelta[Player_Count];
	player_resources TurnStartTapDelta[Player_Count];

	rectangle Rectangle;
	card_set_type SetType;
	float TimeLeft;
	vector4 Color;
	bool Active;
	bool HoveredOver;
	player_id Owner;
	int32_t TapsAvailable;
	int32_t TimesTapped;
	int16_t Attack;
	int16_t TurnStartAttack;
	int16_t Health;
	int16_t TurnStartHealth;
	card_effect_tags EffectTags;
};

struct card_set
{
	card* Cards[MAX_CARDS_PER_SET];
	int CardCount;
	float ScreenWidth;
	float CardWidth;
	float YPos;
	card_set_type Type;
};

struct card_game_state
{
	player_id CurrentTurn;
	int16_t LastWholeSecond;
	float TurnTimer;
	bool ShouldUpdateBaseline;
	float BaselineNextTurnTimer;
	float NextTurnTimer;
	player_resources* PlayerResources;
	card* Cards;
	card* SelectedCard;
	int MaxCards;
	deck* Decks;
	card_set* Hands;
	card_set* Tableaus;
	vector2 InfoCardCenter;
	vector2 InfoCardXBound;
	vector2 InfoCardYBound;
	alert Alert;
	card_definitions* Definitions;
	float CardWidth;
	float CardHeight;
};

struct start_card_game_args
{
	loaded_deck P1Deck;
	loaded_deck P2Deck;
};
void StartCardGamePrep(
	game_state* GameState, char* P1DeckName, char* P2DeckName
);

#define APOCALYPSE_CARD_GAME_H
#endif