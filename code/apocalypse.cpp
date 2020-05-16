#include "apocalypse.h"

#include <math.h>

#define Pi32 3.14159265359f

void GameUpdateAndRender(
	game_memory* Memory,
	game_offscreen_buffer* BackBuffer,
	game_mouse_events* MouseEvents,
	game_sound_output_buffer* SoundBuffer
)
{
	ASSERT(sizeof(game_state) <= Memory->PermanentStorageSize);
	game_state* GameState = (game_state*) Memory->PermanentStorage;
	if(!Memory->IsInitialized)
	{
		GameState->XOffset = 0;
		GameState->YOffset = 0;
		GameState->CurrentPrimaryState = PrimaryUp;
		GameState->SineT = 0;
		GameState->ToneHz = 256;

		// TODO: this may be more appropriate in the platform layer
		Memory->IsInitialized = true;
	}

	// SECTION START: User input
	// TODO: move to game memory
	for(
		int MouseEventIndex = 0;
		MouseEventIndex < MouseEvents->Length;
		MouseEventIndex++
	)
	{
		// TODO: remove last action wins stuff from 
		// NOTE: testing platform layer mouse stuff
		game_mouse_event* MouseEvent = &MouseEvents->Events[MouseEventIndex];

		if(
			MouseEvent->Type == PrimaryDown ||
			MouseEvent->Type == PrimaryUp
		)
		{
			GameState->CurrentPrimaryState = MouseEvent->Type;
		}

		if(GameState->CurrentPrimaryState == PrimaryDown)
		{
			GameState->XOffset = BackBuffer->Width - MouseEvent->XPos;
			GameState->YOffset = BackBuffer->Height - MouseEvent->YPos;
		}
	}
	// SECTION STOP: User input

	// SECTION START: Audio code
	int16_t ToneVolume = 3000;
	int WavePeriod = SoundBuffer->SamplesPerSecond / GameState->ToneHz;
	int16_t* SampleOut = SoundBuffer->Samples;
	for(
		int SampleIndex = 0;
		SampleIndex < SoundBuffer->SampleCount;
		++SampleIndex
	)
	{
		float SineValue = sinf(GameState->SineT);
		int16_t SampleValue = (int16_t) (SineValue * ToneVolume);
		GameState->SineT += (2.0f * Pi32 * 1.0f) / ((float) WavePeriod);

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
			*ColorChannel++ = (uint8_t) (X + GameState->XOffset);
			*ColorChannel++ = (uint8_t) (Y + GameState->YOffset);
			*ColorChannel++ = 0;
			Pixel++;
		}
		Row += BackBuffer->Pitch;
	}
}