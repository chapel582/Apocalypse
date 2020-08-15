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
	int16_t Attack,
	int16_t Health,
	player_resources* PlayDelta,
	player_resources* TapDelta,
	int32_t TapsRemaining
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
	int32_t TapsRemaining
);

#define APOCALYPSE_INFO_CARD_H
#endif