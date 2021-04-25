#ifndef APOCALYPSE_CARD_DEFINITIONS_H

#include "apocalypse_player_id.h"

#include <stdint.h>
#include <string.h>

#define MAX_CARDS_IN_GAME 200

typedef enum
{
	GridEffect_IsGeneral,
	/* 
	NOTE: card attack decreases during owner's turn, resets at end of owner's
	turn
	*/
	GridEffect_SelfWeaken,
	/* 
	NOTE: card attack increases during opponent's turn, resets at end of 
	opponent's turn
	*/
	GridEffect_OppStrengthen,
	/*
	NOTE: card life decreases during owner's turn, resets at end of owner's
	turn
	*/
	GridEffect_SelfLifeLoss,
	/*
	NOTE: card life increases during opponent's turn, resets at end of 
	opponent's turn
	*/
	GridEffect_OppLifeGain,
	/*
	NOTE: cost increases during owner's turn, resets at end of owner's turn
	*/
	GridEffect_CostIncrease,
	/*
	NOTE: time given to opponent increases during owner's turn, resets at end of
	owner's turn
	*/
	GridEffect_GiveIncrease,
	/*
	NOTE: while this card is in the tableau, give turn timer a 10 second boost
	*/
	GridEffect_CurrentBoost,
	/*
	NOTE: while this card is in the tableau, lose 10 seconds from the turn timer
	at the start of your turn
	*/
	GridEffect_CurrentLoss,
	/*
	NOTE: while this card is in the tableau, when you end your turn, your card's
	attack is increased by half of the remaining turn time
	*/
	GridEffect_GainRemainingAsAttack,
	/*
	NOTE: When a taunt card is in play, only taunt cards can be attacked
	*/
	GridEffect_Taunt,
	/*
	NOTE: Trigger the self delta on each attack
	*/
	GridEffect_SelfDeltaOnAttack,
	/*
	NOTE: Trigger the opp delta on each attack
	*/
	GridEffect_OppDeltaOnAttack,
	/*
	NOTE: SelfDelta acts on current instead of next turn
	if this would cause the current timer to go below 0, it cannot be played

	TODO: should we have some way to tag stack cards with this as well?
	*/
	GridEffect_SelfDeltaFromCurrent,
	/*
	NOTE: OppDelta acts on current instead of next turn
	if this would cause current timer to go below 0, it cannot be played

	TODO: should we have some way to tag stack cards with this as well?
	*/
	GridEffect_OppDeltaFromCurrent,

	/*
	TODO: Unimplemented card ideas
	A tag that gives your opponent time on an attack
	A tag that causes a card to require your time to attack
	A tag that has a card switch from attacking to healing over time (like attack loss, but with negative values permitted)
	Expensive cards that give you time each time you play a stack card
	*/
} grid_effect;

struct grid_effect_tags
{
	// NOTE: wrapped in a struct for easy transtion to having even more tags
	uint64_t Tags;
};

inline void SetTag(grid_effect_tags* Tags, grid_effect ToAdd)
{
	ASSERT(ToAdd < ((8 * sizeof(grid_effect_tags)) - 1));
	Tags->Tags |= (1LL << ToAdd);
}

inline bool HasTag(grid_effect_tags* Tags, grid_effect Check)
{
	ASSERT(Check < ((8 * sizeof(grid_effect_tags)) - 1));
	return (Tags->Tags & (1LL << Check)) > 0;
}

typedef enum
{
	StackEffect_NoEffect,
	/*
	NOTE: this effect is boring and should probably only be for testing
	*/
	StackEffect_HurtOpp,
	/*
	NOTE: This is a test effect only
	*/
	StackEffect_DisableNext,
	/*
	NOTE: increases current time for the current player by 20 seconds
	*/
	StackEffect_IncreaseCurrentTime,
	/*
	NOTE: decrease current time by 20 seconds
	*/
	StackEffect_DecreaseCurrentTime,
	/*
	NOTE: swaps the self and opp deltas for any card in either player's hand
	*/
	StackEffect_SwapDeltas,
	/*
	NOTE: randomize the opp deltas for all of your opponent's cards in a range
	from min to max. uniform distribution, integers only
	*/
	StackEffect_OppDeltaConfuse,
	/*
	NOTE: randomize the self deltas for all of your opponent's cards in a range
	from min to max. uniform distribution, integers only
	*/
	StackEffect_SelfDeltaConfuse,
	/*
	NOTE: draw two cards
	*/
	StackEffect_DrawTwo,
	/*
	NOTE: Each player discards a card at random
	*/
	StackEffect_RandomDiscard,
	/*
	NOTE: Each player draws a card
	*/
	StackEffect_BothDraw,
	/*
	NOTE: Any time unused from the current turn goes to the next turn.
	*/
	StackEffect_PassRemaining,
	/*
	NOTE: Any time unused from the next turn goes to the current turn.
	*/
	StackEffect_GetRemaining,
	/*
	NOTE: Discard a card. Trigger the opp delta for the owner instead of the
	opp
	*/
	StackEffect_DiscardAndGive,
	/*
	NOTE: Draw a card. Trigger the cost delta for the owner instead of the
	opponent
	*/
	StackEffect_DrawAndCost


	/*
	TODO: Unimplemented card ideas
	Card that lets a player draw a card at the cost of their next turn time
	Card that lets both players draw a card (at the cost of time for both players)
	Card that lets you reshuffle your deck
	Card that lets you view your top few cards
	Card that lets you draw another card
	Card that makes you shuffle and draw from the discard pile
	Card that avoids a turn skip (played automatically from your hand)
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
#define CARD_DESCRIPTION_SIZE 256
struct card_definition
{
	uint32_t Id;
	int32_t TapsAvailable;
	int16_t Attack;
	int16_t Health;
	int16_t SelfPlayDelta;
	int16_t OppPlayDelta;
	uint8_t Movement;
	char Name[CARD_NAME_SIZE];
	char Description[CARD_DESCRIPTION_SIZE];
	grid_effect_tags GridTags;
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
	Card->Movement = 2;
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