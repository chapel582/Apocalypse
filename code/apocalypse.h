#ifndef APOCALYPSE_H

#include <stdint.h>

#if APOCALYPSE_SLOW
// TODO: Complete assertion macro
#define ASSERT(Expression) if(!(Expression)) {*(int*) 0 = 0;}
#else
#define ASSERT(Expression)
#endif

#define ARRAY_COUNT(Array) (sizeof(Array) / sizeof(Array[0]))

#define KILOBYTES(Value) (1024LL * Value)
#define MEGABYTES(Value) (1024LL * KILOBYTES(Value))
#define GIGABYTES(Value) (1024LL * MEGABYTES(Value))
#define TERABYTES(Value) (1024LL * GIGABYTES(Value))

inline uint32_t SafeTruncateUInt64(uint64_t Value)
{
	ASSERT(Value <= 0xFFFFFFFF);
	return (uint32_t) Value;
}

#if APOCALYPSE_INTERNAL
// NOTE: These are blocking calls that don't protect against lost data
// CONT: they are intended for debug purposes only
struct debug_read_file_result
{
	uint32_t ContentsSize;
	void* Contents;
};

debug_read_file_result DEBUGPlatformReadEntireFile(char* FileName);
void DEBUGPlatformFreeFileMemory(void* Memory);
bool DEBUGPlatformWriteEntireFile(
	char* FileName, void* Memory, uint32_t MemorySize
);
#endif 

struct game_memory
{
	bool IsInitialized;

	size_t PermanentStorageSize;
	void* PermanentStorage;

	size_t TransientStorageSize;
	void* TransientStorage;
};

struct game_offscreen_buffer
{
	void* Memory;
	int Width;
	int Height;
	int Pitch;
	int BytesPerPixel;
};

struct game_sound_output_buffer
{
	int SamplesPerSecond;
	int SampleCount;
	int16_t* Samples;
};

typedef enum
{
	PrimaryDown,
	PrimaryUp,
	SecondaryDown,
	SecondaryUp,
	MouseMove
} mouse_event_type;

// NOTE: user events (mouse and keyboard) have UserEventIndex field
// NOTE: this is to allow them to 
typedef uint16_t user_event_index;
struct game_mouse_event
{
	user_event_index UserEventIndex;
	int XPos;
	int YPos;
	mouse_event_type Type;
};

struct game_mouse_events
{
	// TODO: see if we can drop this below 128?
	game_mouse_event Events[128];
	int Length;
};

// TODO: other platform layers need to conform their key codes
struct game_keyboard_event
{
	user_event_index UserEventIndex;
	uint8_t Code; // NOTE: right now this is just the VK from Windows
	// TODO: see if it would be helpful to combine the two below into a 
	// CONT: single byte with flags
	uint8_t IsDown; // NOTE: compact bool
	uint8_t WasDown; // NOTE: compact bool
};

struct game_keyboard_events
{
	// TODO: see if we can drop this below 1024?
	game_keyboard_event Events[1024];
	int Length;
};

struct keyboard_state
{
	bool ShiftDown;
	bool CtrlDown;
	// TODO: caps lock, other stateful things?
};

void GameUpdateAndRender(
	game_memory* Memory,
	game_offscreen_buffer* BackBuffer,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float dtForFrame
);

void GameFillSound(game_memory* Memory, game_sound_output_buffer* SoundBuffer);

struct card
{
	float PosX;
	float PosY;
	float Width;
	float Height;
	float TimeLeft;
	bool Active;
};

struct game_state
{
	int XOffset;
	int YOffset;

	float SineT;
	int ToneHz;

	char TempBuffer[1024];
	int TempBufferLength;

	mouse_event_type CurrentPrimaryState;

	card Cards[14];
};

#define APOCALYPSE_H
#endif
