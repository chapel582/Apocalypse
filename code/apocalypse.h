#ifndef APOCALYPSE_H

#include "apocalypse_platform.h"
#include "apocalypse_vector.h"
#include "apocalypse_rectangle.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_bitmap.h"
#include "apocalypse_render_group.h"

#define MAX_CARDS_PER_SET 7

typedef enum
{
	Player_One,
	Player_Two,
	Player_Count
} player_e;

typedef enum
{
	CardSet_Hand,
	CardSet_Tableau,
	CardSet_Count
} card_set_type;

struct deck_card;
struct deck_card
{
	int RedCost;
	int GreenCost;
	int BlueCost;
	deck_card* Next;
	deck_card* Previous;
};

#define MAX_CARDS_IN_DECK 60
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
		deck_card* OldLast = First->Previous;
		if(OldLast != NULL)
		{
			OldLast->Next = DeckCard;
		}
		DeckCard->Previous = OldLast;
	}
	else
	{
		DeckCard->Previous = NULL;
	}

	DeckCard->Next = First;
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
}

struct card
{
	rectangle Rectangle;
	float TimeLeft;
	vector4 Color;
	int RedCost;
	int GreenCost;
	int BlueCost;
	bool Active;
	player_e Owner;
};

struct card_set
{
	card* Cards[MAX_CARDS_PER_SET];
	int CardCount;
	float ScreenWidth;
	float CardWidth;
	float YPos;
};

struct game_state
{
	// NOTE: Arena is just for permanent things that can/should be determined
	// CONT: at run time
	memory_arena Arena;
	// NOTE: TransientArena is for things that last longer than a frame but 
	// CONT: can be cleaned up in a big batch. stuff like composite bitmaps
	memory_arena TransientArena;
	// NOTE: FrameArena is just for things that will be dead by EOF
	memory_arena FrameArena;
	// NOTE: render arena is just for the renderer
	memory_arena RenderArena;

	render_group RenderGroup;
	basis WorldToCamera;
	basis CameraToScreen;

	float Time;
	
	// TODO: delete test code below
	loaded_bitmap TestBitmap;
	loaded_bitmap TestCard;
	loaded_bitmap TestBackground;
	int ToneHz;
	mouse_event_type CurrentPrimaryState;
	loaded_bitmap TestDiffuse;
	loaded_bitmap TestNormal;
	int EnvMapWidth;
	int EnvMapHeight;
	// NOTE: 0 is bottom, 1 is middle, 2 is top
	environment_map EnvMaps[3];
	// TODO: done with test code here

	// SECTION START: Card Game Code
	card* Cards;
	int MaxCards;
	deck* Decks;
	card_set* Hands;
	card_set* Tableaus;
	// SECTION STOP: Card GameCode

};

#define APOCALYPSE_H
#endif
