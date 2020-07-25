#ifndef APOCALYPSE_H

#include "apocalypse_platform.h"
#include "apocalypse_vector.h"
#include "apocalypse_rectangle.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_bitmap.h"
#include "apocalypse_render_group.h"
#include "apocalypse_assets.h"
#include "apocalypse_audio.h"
#include "apocalypse_particles.h"

#define MAX_CARDS_PER_SET 7

typedef enum
{
	Player_One,
	Player_Two,
	Player_Count
} player_id;

typedef enum
{
	CardSet_Hand,
	CardSet_Tableau,
	CardSet_Count
} card_set_type;

typedef enum 
{
	PlayerResource_Red,
	PlayerResource_Green,
	PlayerResource_Blue,
	PlayerResource_White,
	PlayerResource_Black,
	PlayerResource_Count	
} player_resource_type;

struct player_resources
{
	int32_t Resources[PlayerResource_Count];
};

inline bool CanChangeResources(
	player_resources* Target, player_resources* Delta
)
{
	for(int Index = 0; Index < PlayerResource_Count; Index++)
	{
		if(Target->Resources[Index] + Delta->Resources[Index] < 0)
		{
			return false;
		}
	}
	return true;
}

inline void ChangeResources(player_resources* Target, player_resources* Delta)
{
	for(int Index = 0; Index < PlayerResource_Count; Index++)
	{
		Target->Resources[Index] += Delta->Resources[Index];
		if(Target->Resources[Index] <= 0)
		{
			Target->Resources[Index] = 0;
		}
	}
}

inline void SetResource(
	player_resources* Resources, player_resource_type Type, int32_t SetTo
)
{
	Resources->Resources[Type] = SetTo;
}

struct deck_card;
struct deck_card
{
	player_resources PlayDelta[Player_Count];
	player_resources TapDelta[Player_Count];
	int32_t TapsAvailable;
	int16_t Attack;
	int16_t Health;
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
	rectangle Rectangle;
	card_set_type SetType;
	float TimeLeft;
	vector4 Color;
	player_resources PlayDelta[Player_Count];
	player_resources TapDelta[Player_Count];
	bool Active;
	bool HoveredOver;
	player_id Owner;
	int32_t TapsAvailable;
	int32_t TimesTapped;
	int16_t Attack;
	int16_t Health;
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

struct game_state
{
	// NOTE: Arena is just for permanent things that can/should be determined
	// CONT: at run time
	memory_arena Arena;
	// NOTE: TransientArena is for memory that last longer than a frame but 
	// CONT: can be cleaned up in a big batch, usually when switching contexts 
	// CONT: in the game, e.g. from the menu to starting the game 
	memory_arena TransientArena;
	// NOTE: FrameArena is just for memory that will not be needed after EOF
	memory_arena FrameArena;
	// NOTE: render arena is just for the renderer
	memory_arena RenderArena;

	assets Assets;

	render_group RenderGroup;
	basis WorldToCamera;
	basis CameraToScreen;

	playing_sound_list PlayingSoundList;

	float Time;

	bool OverlayDebugInfo;

	// TODO: remove test particle system
	particle_system TestParticleSystem;

	// SECTION START: Card Game Code
	player_id CurrentTurn;
	float TurnTimer;
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
	float DisplayMessageUntil;
	char MessageBuffer[256];
	// SECTION STOP: Card GameCode
};

#define APOCALYPSE_H
#endif
