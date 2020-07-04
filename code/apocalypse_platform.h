#ifndef APOCALYPSE_PLATFORM_H

#if _MSC_VER
#include <intrin.h>
#endif

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

#define BYTES_PER_PIXEL 4 

inline uint32_t SafeTruncateUInt64(uint64_t Value)
{
	ASSERT(Value <= 0xFFFFFFFF);
	return (uint32_t) Value;
}

// TODO: remove DEBUG IO
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

#if APOCALYPSE_INTERNAL
enum
{
	DebugCycleCounter_GameUpdateAndRender,
	DebugCycleCounter_RenderGroupToOutput,
	DebugCycleCounter_DrawBitmapQuickly,
	DebugCycleCounter_ProcessPixel,
	DebugCycleCounter_MixAudio,
	DebugCycleCounter_MixAudio_Init,
	DebugCycleCounter_MixAudioSample,
	DebugCycleCounter_SetAudioSample,
	DebugCycleCounter_Count
};

struct debug_cycle_counter
{
	uint64_t CycleCount;
	uint32_t HitCount;
};

extern struct game_memory* DebugGlobalMemory;

#if _MSC_VER
#define BEGIN_TIMED_BLOCK(ID) uint64_t StartCycleCount##ID = __rdtsc(); 
#define END_TIMED_BLOCK(ID) DebugGlobalMemory->Counters[DebugCycleCounter_##ID].CycleCount += __rdtsc() - StartCycleCount##ID; DebugGlobalMemory->Counters[DebugCycleCounter_##ID].HitCount++;
#define END_TIMED_BLOCK_COUNTED(ID, Count) DebugGlobalMemory->Counters[DebugCycleCounter_##ID].CycleCount += __rdtsc() - StartCycleCount##ID; DebugGlobalMemory->Counters[DebugCycleCounter_##ID].HitCount += (Count);
#else
#define BEGIN_TIMED_BLOCK(ID) 
#define END_TIMED_BLOCK(ID)
#define END_TIMED_BLOCK_COUNTED(ID, Count)
#endif

#else // NOTE: !APOCALYPSE_INTERNAL
#define BEGIN_TIMED_BLOCK(ID) 
#define END_TIMED_BLOCK(ID)
#define END_TIMED_BLOCK_COUNTED(ID, Count)
#endif // NOTE: APOCALYPSE_INTERNAL

// SECTION START: Threading Code
typedef void platform_job_callback(void* Data);

struct platform_job_queue_entry;
struct platform_job_queue_entry
{
	platform_job_callback* Callback;
	void* Data;

	platform_job_queue_entry* Next;
};

struct platform_mutex_handle;
struct platform_semaphore_handle;
struct platform_event_handle;
struct platform_job_queue
{
	platform_job_queue_entry Entries[256];
	platform_event_handle* JobDone;
	platform_semaphore_handle* EmptySemaphore;
	platform_semaphore_handle* FilledSemaphore;
	platform_mutex_handle* UsingFilled; // NOTE: for making mods to FilledQueue
	platform_mutex_handle* UsingEmpty; // NOTE: for making mods to EmptyQueue
	platform_job_queue_entry* FilledHead; // NOTE: Jobs to do
	platform_job_queue_entry* EmptyHead; // NOTE: entries available for jobs
};

void PlatformAddJob(
	platform_job_queue* JobQueue,
	platform_job_callback* Callback,
	void* Data
);
void PlatformCompleteAllJobs(platform_job_queue* JobQueue);
// SECTION STOP: Threading Code

struct game_memory
{
	bool IsInitialized;

	size_t PermanentStorageSize;
	void* PermanentStorage;

	size_t TransientStorageSize;
	void* TransientStorage;

	platform_job_queue* DefaultJobQueue;

#if APOCALYPSE_INTERNAL
	debug_cycle_counter Counters[DebugCycleCounter_Count];
#endif
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
	uint32_t SampleCount;
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
	int XPos; // NOTE: should be xpos from bottom left
	int YPos; // NOTE: should be xpos from bottom left
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

#define APOCALYPSE_PLATFORM_H
#endif