#ifndef APOCALYPSE_WAV_H

#include <stdint.h>

struct loaded_wav
{
	uint32_t SampleCount;
	uint32_t ChannelCount;
	// NOTE: samples are actually 16 bit, but we store them as 32 bit for
	// CONT: easy mixing
	int16_t* Samples[2];
};

#define APOCALYPSE_WAV_H
#endif