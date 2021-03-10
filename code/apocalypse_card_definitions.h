#ifndef APOCALYPSE_CARD_DEFINITIONS_H

#include "apocalypse_player_id.h"

#include <stdint.h>
#include <string.h>

#define MAX_CARDS_IN_GAME 200

typedef enum
{
	/* 
	NOTE: card attack decreases during owner's turn, resets at end of owner's
	turn
	*/
	TableauEffect_SelfWeaken,
	/* 
	NOTE: card attack increases during opponent's turn, resets at end of 
	opponent's turn
	*/
	TableauEffect_OppStrengthen,
	/*
	NOTE: card life decreases during owner's turn, resets at end of owner's
	turn
	*/
	TableauEffect_SelfLifeLoss,
	/*
	NOTE: card life increases during opponent's turn, resets at end of 
	opponent's turn
	*/
	TableauEffect_OppLifeGain,
	/*
	NOTE: cost increases during owner's turn, resets at end of owner's turn
	*/
	TableauEffect_CostIncrease,
	/*
	NOTE: time given to opponent increases during owner's turn, resets at end of
	owner's turn
	*/
	TableauEffect_GiveIncrease

	/*
	TODO: Unimplemented card ideas
	A tag that gives your opponent time on an attack
	A tag that causes a card to require your time to attack
	A tag that has a card switch from attacking to healing over time (like attack loss, but with negative values permitted)
	*/
} tableau_effect;

struct tableau_effect_tags
{
	// NOTE: wrapped in a struct for easy transtion to having even more tags
	uint64_t Tags;
};

inline void SetTag(tableau_effect_tags* Tags, tableau_effect ToAdd)
{
	ASSERT(ToAdd < ((8 * sizeof(tableau_effect_tags)) - 1));
	Tags->Tags |= (1LL << ToAdd);
}

inline bool HasTag(tableau_effect_tags* Tags, tableau_effect Check)
{
	ASSERT(Check < ((8 * sizeof(tableau_effect_tags)) - 1));
	return (Tags->Tags & (1LL << Check)) > 0;
}

typedef enum
{
	/*
	NOTE: this effect is boring and should probably only be for testing
	*/
	StackEffect_HurtOpp,
	/*
	NOTE: This is a test effect only
	*/
	StackEffect_DisableNext
	/*
	TODO: Unimplemented card ideas
	Card that lets a player draw a card at the cost of their next turn time
	Card that lets both players draw a card (at the cost of time for both players)
	Card that lets you reshuffle your deck
	Card that lets you view your top few cards
	Card that lets you draw another card
	Card that makes you shuffle and draw from the discard pile
	*/
} stack_effect;

struct stack_effect_tags
{
	uint64_t Tags;
};

inline void SetTag(stack_effect_tags* Tags, stack_effect ToAdd)
{
	ASSERT(ToAdd < (8 * sizeof(stack_effect_tags) - 1));
	Tags->Tags |= (1LL << ToAdd);
}

inline bool HasTag(stack_effect_tags* Tags, stack_effect Check)
{
	ASSERT(Check < (8 * sizeof(stack_effect_tags) - 1));
	return (Tags->Tags & (1LL << Check)) > 0;
}

inline bool HasAnyTag(stack_effect_tags* Tags)
{
	return Tags->Tags != 0;
}

#define CARD_NAME_SIZE 64
#define CARD_DESCRIPTION_SIZE 128
struct card_definition
{
	uint32_t Id;
	int32_t TapsAvailable;
	int16_t Attack;
	int16_t Health;
	int16_t SelfPlayDelta;
	int16_t OppPlayDelta;
	char Name[CARD_NAME_SIZE];
	char Description[CARD_DESCRIPTION_SIZE];
	tableau_effect_tags TableauTags;
	stack_effect_tags StackTags;
};

struct card_definitions
{
	uint32_t NumCards;
	card_definition* Array;
};

inline card_definition* GetCardDefinition(
	card_definitions* Definitions, uint32_t DefId
)
{
	ASSERT(DefId < Definitions->NumCards);
	return Definitions->Array + DefId;
}

inline void InitCard(
	card_definition* Card,
	uint32_t Id,
	int32_t TapsAvailable,
	int16_t Attack,
	int16_t Health,
	int16_t SelfPlayDelta,
	int16_t OppPlayDelta
)
{
	*Card = {};
	Card->Id = Id;
	Card->TapsAvailable = TapsAvailable;
	Card->Attack = Attack;
	Card->Health = Health;
	Card->SelfPlayDelta = SelfPlayDelta;
	Card->OppPlayDelta = OppPlayDelta;
}

inline void SetName(card_definition* Definition, char* Name, uint32_t NameSize)
{
	// TODO: replace with memcpy_s
	memcpy_s(Definition->Name, CARD_NAME_SIZE, Name, NameSize);
}

inline void SetDescription(
	card_definition* Definition, char* Description, uint32_t DescriptionSize
)
{
	// TODO: replace with memcpy_s
	memcpy_s(
		Definition->Description,
		CARD_DESCRIPTION_SIZE,
		Description,
		DescriptionSize
	);
}

card_definitions* DefineCards(memory_arena* MemoryArena);

#define APOCALYPSE_CARD_DEFINITIONS_H
#endif