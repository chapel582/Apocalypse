#ifndef APOCALYPSE_INFO_CARD_H

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

#define APOCALYPSE_INFO_CARD_H
#endif