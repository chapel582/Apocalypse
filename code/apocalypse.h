#ifndef APOCALYPSE_H

#include "apocalypse_platform.h"

struct memory_arena
{
	size_t TotalSize;
	uint8_t* Base;
	size_t Used;
};

void InitMemArena(memory_arena* Arena, size_t TotalSize, uint8_t* Base)
{
	Arena->TotalSize = TotalSize;
	Arena->Base = Base;
	Arena->Used = 0;
}

#define PushStruct(Arena, Type) (Type*) PushSize(Arena, sizeof(Type))
#define PushArray(Arena, Count, Type) (Type*) PushSize(Arena, (Count) * sizeof(Type))
void* PushSize(memory_arena* Arena, size_t SizeToPush)
{
	void* Result = Arena->Base + Arena->Used;
	Arena->Used += SizeToPush;
	ASSERT(Arena->Used <= Arena->TotalSize);
	return Result;
}

struct loaded_bitmap
{
	int32_t Width;
	int32_t Height;
	uint32_t* Pixels;
};

// TODO: move this out to a math file
// SECTION START: Vector2
struct vector2
{
	float X;
	float Y;
};

vector2 Vector2(float X, float Y)
{
	vector2 Result = {};
	Result.X = X;
	Result.Y = Y;
	return Result;
}
// SECTION STOP: Vector2

struct world_screen_converter
{
	float ScreenToWorld;
	float WorldToScreen;
	// NOTE: ScreenYOffset is where the screen origin is relative to the 
	// CONT: World origin but in pixels
	float ScreenYOffset; 
	// NOTE: WorldYOffset is where the world origin is relative to the Screen 
	// CONT: origin but in world units
	float WorldYOffset;
};

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
	*ToDecrement--;

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
	*ToIncrement++;
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
	vector2 Pos; // NOTE: Bottom left
	vector2 Dim;
	float TimeLeft;
	float Red;
	float Green;
	float Blue;
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
	memory_arena Arena;

	loaded_bitmap TestBitmap;
	
	float SineT;
	int ToneHz;

	mouse_event_type CurrentPrimaryState;

	world_screen_converter WorldScreenConverter;
	
	card* Cards;
	int MaxCards;
	deck* Decks;
	card_set* Hands;
	card_set* Tableaus;
};

#define APOCALYPSE_H
#endif
