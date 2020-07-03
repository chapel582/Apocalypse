#ifndef APOCALYPSE_WAV_H

#include <stdint.h>

struct loaded_wav
{
	uint32_t SampleCount;
	uint32_t ChannelCount;
	int16_t* Samples[2];
};

#define APOCALYPSE_WAV_H
#endif