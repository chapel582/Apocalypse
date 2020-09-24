#ifndef APOCALYPSE_MEMORY_ARENA_H

struct memory_arena
{
	size_t TotalSize;
	uint8_t* Base;
	size_t Used;
};

void InitMemArena(memory_arena* Arena, size_t TotalSize, uint8_t* Base)
{
	Arena->TotalSize = TotalSize;
	Arena->Base = Base;
	Arena->Used = 0;
}

#define FindAlignmentOffset(Pointer, Alignment) FindAlignmentOffset_((size_t) Pointer, Alignment)
inline size_t FindAlignmentOffset_(size_t Pointer, size_t Alignment)
{
	size_t AlignmentMask = Alignment - 1;
	size_t Misalignment = Pointer & AlignmentMask;
	size_t AlignmentOffset;
	if(Misalignment)
	{
		AlignmentOffset = Alignment - Misalignment;
	}
	else
	{
		AlignmentOffset = 0;
	}

	return AlignmentOffset;
}

#define DEFAULT_ALIGNMENT 4
#define PushStruct(Arena, Type, ...) (Type*) PushSize(Arena, sizeof(Type), ##__VA_ARGS__)
#define PushArray(Arena, Count, Type, ...) (Type*) PushSize(Arena, (Count) * sizeof(Type), ##__VA_ARGS__)
inline void* PushSize(
	memory_arena* Arena, size_t SizeToPush, size_t Alignment = DEFAULT_ALIGNMENT
)
{
	// NOTE: find alignment
	size_t ResultPointer = ((size_t) Arena->Base) + Arena->Used; 
	size_t AlignmentOffset = FindAlignmentOffset(ResultPointer, Alignment);

	// NOTE: return new pointer for use
	void* Result = Arena->Base + Arena->Used + AlignmentOffset;
	Arena->Used += SizeToPush + AlignmentOffset;
	ASSERT(Arena->Used <= Arena->TotalSize);
	return Result;
}

inline uint8_t* GetEndOfArena(memory_arena* Arena)
{
	return Arena->Base + Arena->TotalSize;
}

inline void ResetMemArena(memory_arena* Arena)
{
	Arena->Used = 0;
}

struct temp_memory
{
	memory_arena* Arena;
	size_t OldUsed;
};

inline temp_memory BeginTempMemory(memory_arena* Arena)
{
	temp_memory TempMemory = {};
	TempMemory.Arena = Arena;
	TempMemory.OldUsed = Arena->Used;
	return TempMemory;
}

inline void EndTempMemory(temp_memory TempMemory)
{
	TempMemory.Arena->Used = TempMemory.OldUsed;
}

#define APOCALYPSE_MEMORY_ARENA_H
#endif