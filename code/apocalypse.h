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

struct game_state
{
	int XOffset;
	int YOffset;

	float SineT;
	int ToneHz;

	mouse_event_type CurrentPrimaryState;
};
#define APOCALYPSE_H
#endif
