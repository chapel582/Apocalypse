#ifndef APOCALYPSE_SEQUENCE_ALIGNMENT_H

#include "apocalypse_memory_arena.h"

#include <stdint.h>

float* AllocScoreMemory(
	memory_arena* Arena,
	uint32_t MaxString1Size, 
	uint32_t MaxString2Size
);
float SequenceAlignmentScore(
	char* String1,
	uint32_t String1Size,
	char* String2,
	uint32_t String2Size,
	float* ScoreMemory
);

#define APOCALYPSE_SEQUENCE_ALIGNMENT_H
#endif