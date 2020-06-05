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

#define PushStruct(Arena, Type) (Type*) PushSize(Arena, sizeof(Type))
#define PushArray(Arena, Count, Type) (Type*) PushSize(Arena, (Count) * sizeof(Type))
void* PushSize(memory_arena* Arena, size_t SizeToPush)
{
	void* Result = Arena->Base + Arena->Used;
	Arena->Used += SizeToPush;
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