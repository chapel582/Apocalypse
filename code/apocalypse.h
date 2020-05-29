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

// TODO: move this out to a math file
// SECTION START: Vector2
struct vector2
{
	float X;
	float Y;
};

vector2 Vector2(float X, float Y)
{
	vector2 Result = {};
	Result.X = X;
	Result.Y = Y;
	return Result;
}
// SECTION STOP: Vector2

struct card
{
	vector2 Pos; // NOTE: Bottom left
	vector2 Dim;
	float TimeLeft;
	float Red;
	float Green;
	float Blue;
	bool Active;
};

struct world_screen_converter
{
	float ScreenToWorld;
	float WorldToScreen;
	// NOTE: ScreenYOffset is where the screen origin is relative to the 
	// CONT: World origin but in pixels
	float ScreenYOffset; 
	// NOTE: WorldYOffset is where the world origin is relative to the Screen 
	// CONT: origin but in world units
	float WorldYOffset;
};

#define MAX_CARDS_PER_SET 7

typedef enum
{
	Player_One,
	Player_Two,
	Player_Count
} player_e;

typedef enum
{
	CardSet_Hand,
	CardSet_Tableau,
	CardSet_Count
} card_set_e;

struct card_set_s
{
	card* Cards[MAX_CARDS_PER_SET];
	int CardCount;
	float ScreenWidth;
	float YPos;
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

	world_screen_converter WorldScreenConverter;
	
	card Cards[Player_Count * CardSet_Count * MAX_CARDS_PER_SET];
	card_set_s Hands[Player_Count];
	card_set_s Tableaus[Player_Count];
};

#define APOCALYPSE_H
#endif
