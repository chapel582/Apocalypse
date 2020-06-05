#ifndef APOCALYPSE_H

#include "apocalypse_platform.h"
#include "apocalypse_vector.h"

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

inline uint8_t* GetEndOfArena(memory_arena* Arena)
{
	return Arena->Base + Arena->TotalSize;
}

inline void ResetMemArena(memory_arena* Arena)
{
	Arena->Used = 0;
}

struct loaded_bitmap
{
	int32_t Width;
	int32_t Height;
	uint32_t* Pixels;
};

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

struct rectangle
{
	vector2 Min; // NOTE: Bottom left by convention
	vector2 Dim;
};

inline rectangle MakeRectangle(vector2 Min, vector2 Dim)
{
	rectangle Result;
	Result.Min = Min;
	Result.Dim = Dim;
	return Result;
}

inline vector2 GetTopLeft(rectangle Rectangle)
{
	vector2 Result;
	Result.X = Rectangle.Min.X;
	Result.Y = Rectangle.Min.Y + Rectangle.Dim.Y;
	return Result;
}

struct card
{
	rectangle Rectangle;
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

	// TODO: delete test code below
	loaded_bitmap TestBitmap;
	float SineT;
	int ToneHz;
	mouse_event_type CurrentPrimaryState;
	// TODO: done with test code here

	world_screen_converter WorldScreenConverter;
	// NOTE: CameraPos is where the bottom left of camera rectangle is in the
	// CONT: world
	vector2 CameraPos; 

	card* Cards;
	int MaxCards;
	deck* Decks;
	card_set* Hands;
	card_set* Tableaus;
};

#define APOCALYPSE_H
#endif
