#ifndef APOCALYPSE_PLATFORM_H

#if _MSC_VER
#include <intrin.h>
#endif

#include <stdint.h>

// NOTE: compiler flags
#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif
    
#if !defined(COMPILER_LLVM)
#define COMPILER_LLVM 0
#endif

#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif

#if APOCALYPSE_SLOW
// TODO: Complete assertion macro
#define ASSERT(Expression) if(!(Expression)) {*(int*) 0 = 0;}
#else
#define ASSERT(Expression)
#endif

#define ARRAY_COUNT(Array) (sizeof(Array) / sizeof(Array[0]))

#define MAX_THREAD_COUNT 64

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

// NOTE: These are blocking calls that don't protect against lost data
// CONT: they are intended for debug purposes only

// TODO: handle non-windows max path
#define PLATFORM_MAX_PATH 260

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
// TODO: PlatformFindAllFiles needs error handling
void PlatformFindAllFiles(
	char* FilePattern, char* FileNames, uint32_t FileNamesSize 
);
// TODO: error handling
void PlatformDeleteFile(char* FileName);

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
	uint32_t ThreadIds[MAX_THREAD_COUNT];
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
	MouseMove,
	MouseWheel
} mouse_event_type;

// NOTE: user events (mouse and keyboard) have UserEventIndex field
// NOTE: this is to allow them to make sure all user events are ordered
typedef uint16_t user_event_index;

struct game_mouse_event
{
	user_event_index UserEventIndex;
	int XPos; // NOTE: should be xpos from bottom left of screen
	int YPos; // NOTE: should be ypos from bottom left of screen
	mouse_event_type Type;

	//NOTE: Wheel delta is a normalized float between -1 and 1 

	// TODO: see what a good way is to combine this across platforms
	// TODO: if you need to add more data specific to a mouse event, see if we
	// CONT: need to have a union here
	float WheelScroll; 
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
	// TODO: maybe just make IsDown, WasDown flags in a uint8
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
	uint32_t WindowWidth,
	uint32_t WindowHeight,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float dtForFrame
);

void GameFillSound(game_memory* Memory, game_sound_output_buffer* SoundBuffer);
void HandleGameDebug(
	game_memory* Memory, uint32_t WindowWidth, uint32_t WindowHeight
);

#define APOCALYPSE_PLATFORM_H
#endif