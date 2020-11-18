#ifndef APOCALYPSE_CARD_DEFINITIONS_H

#include "apocalypse_player_id.h"
#include "apocalypse_player_resources.h"

#include <stdint.h>
#include <string.h>

#define MAX_CARDS_IN_GAME 200

typedef enum
{
	TableauEffect_Land,
	TableauEffect_SelfWeaken,
	TableauEffect_OppStrengthen,
	TableauEffect_SelfLifeLoss,
	TableauEffect_OppLifeGain,
	TableauEffect_CostIncrease,
	TableauEffect_GiveIncrease,
	TableauEffect_SelfBurn,
	TableauEffect_OppBurn,
	TableauEffect_DrawExtra,
	TableauEffect_DrawOppExtra,
	TableauEffect_GiveTime,
	TableauEffect_GetTime,
	TableauEffect_SelfHandWeaken,
	TableauEffect_TimeGrowth,
	TableauEffect_AttackTimer
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
	StackEffect_HurtOpp,
	StackEffect_DisableNext
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
	player_resources PlayDelta[Player_Count];
	player_resources TapDelta[Player_Count];
	int32_t TapsAvailable;
	int16_t Attack;
	int16_t Health;
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
	int16_t Health
)
{
	*Card = {};
	Card->Id = Id;
	Card->TapsAvailable = TapsAvailable;
	Card->Attack = Attack;
	Card->Health = Health;
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