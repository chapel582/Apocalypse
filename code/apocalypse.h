#ifndef APOCALYPSE_H

#include <stdint.h>

struct game_sound_output_buffer
{
	int SamplesPerSecond;
	int SampleCount;
	int16_t* Samples;
};

struct game_offscreen_buffer
{
	void* Memory;
	int Width;
	int Height;
	int Pitch;
};

typedef enum
{
	PrimaryDown,
	PrimaryUp,
	SecondaryDown,
	SecondaryUp,
	MouseMove
} mouse_event_type;

struct game_mouse_event
{
	int XPos;
	int YPos;
	mouse_event_type Type;
};

struct game_mouse_events
{
	game_mouse_event Events[128];
	int Length;
};

void GameUpdateAndRender(
	game_offscreen_buffer* BackBuffer,
	game_mouse_events* MouseEvents,
	game_sound_output_buffer* SoundBuffer
);

#define APOCALYPSE_H
#endif
