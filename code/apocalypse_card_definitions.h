#ifndef APOCALYPSE_CARD_DEFINITIONS_H

#include "apocalypse_player_id.h"
#include "apocalypse_player_resources.h"

#include <stdint.h>

struct card_definition
{
	uint32_t Id;
	player_resources PlayDelta[Player_Count];
	player_resources TapDelta[Player_Count];
	int32_t TapsAvailable;
	int16_t Attack;
	int16_t Health;
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

#define APOCALYPSE_CARD_DEFINITIONS_H
#endif