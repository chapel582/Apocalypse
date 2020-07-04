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

#define PushStruct(Arena, Type, ...) (Type*) PushSize(Arena, sizeof(Type), ##__VA_ARGS__)
#define PushArray(Arena, Count, Type, ...) (Type*) PushSize(Arena, (Count) * sizeof(Type), ##__VA_ARGS__)
inline void* PushSize(
	memory_arena* Arena, size_t SizeToPush, size_t Alignment = 1
)
{
	// NOTE: find alignment
	size_t ResultPointer = ((size_t) Arena->Base) + Arena->Used; 
	size_t AlignmentMask = Alignment - 1;
	size_t Misalignment = ResultPointer & AlignmentMask;
	size_t AlignmentOffset;
	if(Misalignment)
	{
		AlignmentOffset = Alignment - Misalignment;
	}
	else
	{
		AlignmentOffset = 0;
	}

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

#define APOCALYPSE_MEMORY_ARENA_H
#endif