#ifndef APOCALYPSE_INFO_CARD_H

#include "apocalypse_vector.h"
#include "apocalypse_render_group.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_card_definitions.h"
#include "apocalypse_platform.h"

void PushInfoCard(
	render_group* RenderGroup,
	assets* Assets,
	vector2 InfoCardCenter,
	vector2 InfoCardXBound,
	vector2 InfoCardYBound,
	vector4 Color,
	memory_arena* FrameArena,
	char* Name,
	int16_t Attack,
	int16_t Health,
	int16_t SelfPlayDelta,
	int16_t OppPlayDelta,
	char* Description,
	int32_t TapsRemaining,
	uint32_t Layer = 1
);
void PushInfoCard(
	render_group* RenderGroup,
	assets* Assets,
	vector2 InfoCardCenter,
	vector2 InfoCardXBound,
	vector2 InfoCardYBound,
	vector4 Color,
	memory_arena* FrameArena,
	card_definition* Definition,
	int32_t TapsRemaining,
	uint32_t Layer = 1
);

#define APOCALYPSE_INFO_CARD_H
#endif