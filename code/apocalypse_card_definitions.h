#ifndef APOCALYPSE_CARD_DEFINITIONS_H

#include "apocalypse_player_id.h"
#include "apocalypse_player_resources.h"

#include <stdint.h>
#include <string.h>

#define CARD_NAME_SIZE 64
struct card_definition
{
	uint32_t Id;
	player_resources PlayDelta[Player_Count];
	player_resources TapDelta[Player_Count];
	int32_t TapsAvailable;
	int16_t Attack;
	int16_t Health;
	char Name[CARD_NAME_SIZE];
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
	Card->Id = Id;
	Card->TapsAvailable = TapsAvailable;
	Card->Attack = Attack;
	Card->Health = Health;
}

inline void SetName(card_definition* Definition, char* Name, uint32_t NameSize)
{
	memset(Definition->Name, 0, NameSize);
	memcpy(Definition->Name, Name, NameSize);
}

card_definitions* DefineCards(memory_arena* MemoryArena);

#define APOCALYPSE_CARD_DEFINITIONS_H
#endif