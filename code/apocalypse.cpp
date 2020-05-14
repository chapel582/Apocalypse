#include "apocalypse.h"

#include <math.h>

#define Pi32 3.14159265359f

void GameUpdateAndRender(
	game_offscreen_buffer* BackBuffer, int XOffset, int YOffset,
	game_sound_output_buffer* SoundBuffer, int ToneHz
)
{
	// SECTION START: Audio code
	static float SineT;
	int16_t ToneVolume = 3000;
	int WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;
	int16_t* SampleOut = SoundBuffer->Samples;
	for(
		int SampleIndex = 0;
		SampleIndex < SoundBuffer->SampleCount;
		++SampleIndex
	)
	{
		float SineValue = sinf(SineT);
		int16_t SampleValue = (int16_t) (SineValue * ToneVolume);
		SineT += (2.0f * Pi32 * 1.0f) / ((float) WavePeriod);

		// NOTE: SampleOut writes left and right channels
		*SampleOut++ = SampleValue;
		*SampleOut++ = SampleValue;
	}
	// SECTION STOP: Audio code

	// NOTE: this is currently our render loop
	// CONT: it will be removed soon
	uint8_t* Row = (uint8_t*) BackBuffer->Memory;
	for(int Y = 0; Y < BackBuffer->Height; Y++)
	{
		uint32_t* Pixel = (uint32_t*) Row;
		for(int X = 0; X < BackBuffer->Width; X++)
		{
			// *Pixel = 0x000000;
			uint8_t* ColorChannel = (uint8_t*) Pixel;
			*ColorChannel++ = (uint8_t) (X + XOffset);
			*ColorChannel++ = (uint8_t) (Y + YOffset);
			*ColorChannel++ = 0;
			Pixel++;
		}
		Row += BackBuffer->Pitch;
	}
}