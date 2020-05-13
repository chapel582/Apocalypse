#ifndef APOCALYPSE_H

struct game_offscreen_buffer
{
	void* Memory;
	int Width;
	int Height;
	int Pitch;
};

void GameUpdateAndRender(
	game_offscreen_buffer BackBuffer, int XOffset, int YOffset
);

typedef enum
{
	PrimaryDown,
	PrimaryUp,
	SecondaryDown,
	SecondaryUp,
	MouseMove
} mouse_event_type;

struct apocalypse_mouse_event
{
	int XPos;
	int YPos;
	mouse_event_type Type;
};

struct apocalypse_mouse_events
{
	apocalypse_mouse_event Events[128];
	int Length;
};

#define APOCALYPSE_H
#endif