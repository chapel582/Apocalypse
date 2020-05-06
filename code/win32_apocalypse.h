#ifndef WIN32_APOCALYPSE_H

typedef enum
{
	WindowsSuccess,

	// InitDSound
	DSoundLibLoad,
	DSoundCreate,
	DSoundCooperative,
	DSoundPrimary,
	DSoundSecondary
} windows_result_code;

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

#define WIN32_APOCALYPSE_H
#endif