#ifndef APOCALYPSE_CARD_DEFINITIONS_H

#include "apocalypse_player_id.h"
#include "apocalypse_player_resources.h"

#include <stdint.h>
#include <string.h>

typedef enum
{
	CardEffect_Land,
	CardEffect_SelfWeaken,
	CardEffect_OppStrengthen,
	CardEffect_SelfLifeLoss
} card_effect;

struct card_effect_tags
{
	// NOTE: wrapped in a struct for easy transtion to having even more tags
	uint64_t Tags;
};

inline void SetTag(card_effect_tags* Tags, card_effect ToAdd)
{
	ASSERT(ToAdd < ((8 * sizeof(card_effect_tags)) - 1));
	Tags->Tags |= (1LL << ToAdd);
}

inline bool HasTag(card_effect_tags* Tags, card_effect Check)
{
	return (Tags->Tags & (1LL << Check)) > 0;
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
	card_effect_tags Tags;
};

struct card_definitions
{
	uint32_t NumCards;
	card_definition* Array;
};

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