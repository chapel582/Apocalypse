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
#define PI32 3.14159265359f // TODO: find a better place for this?

inline uint32_t SafeTruncateUInt64(uint64_t Value)
{
	ASSERT(Value <= 0xFFFFFFFF);
	return (uint32_t) Value;
}

// TODO: remove DEBUG IO
// NOTE: These are blocking calls that don't protect against lost data
// CONT: they are intended for debug purposes only
typedef enum
{
	PlatformReadFileResult_Success,
	PlatformReadFileResult_Failure
	// TODO: handle file DNE, permission errors, etc.
} platform_read_file_result;
struct platform_file_handle
{
	// TODO: have a way to keep a file handle open
};

platform_read_file_result PlatformGetFileSize(
	char* FileName, uint32_t* FileSize
);
platform_read_file_result PlatformReadFile(
	char* FileName, void* Contents, uint32_t BytesToRead
);
bool PlatformWriteEntireFile(
	char* FileName, void* Memory, uint32_t MemorySize
);

#if APOCALYPSE_INTERNAL

#if _MSC_VER
#define PLATFORM_CYCLE_COUNT() (__rdtsc())
#else
#define PLATFORM_CYCLE_COUNT() 
#endif

#else // NOTE: !APOCALYPSE_INTERNAL
#endif // NOTE: APOCALYPSE_INTERNAL

// SECTION START: Threading Code
struct platform_mutex_handle;
struct platform_semaphore_handle;
struct platform_event_handle;

platform_mutex_handle* PlatformCreateMutex();
void PlatformGetMutex(platform_mutex_handle* MutexHandle);
void PlatformReleaseMutex(platform_mutex_handle* MutexHandle);
void PlatformFreeMutex(platform_mutex_handle* MutexHandle);

typedef void platform_job_callback(void* Data);

struct platform_job_queue_entry;
struct platform_job_queue_entry
{
	platform_job_callback* Callback;
	void* Data;

	platform_job_queue_entry* Next;
};

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
void HandleGameDebug(game_memory* Memory, game_offscreen_buffer* BackBuffer);

#define APOCALYPSE_PLATFORM_H
#endif