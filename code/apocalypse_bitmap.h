#ifndef APOCALYPSE_BITMAP_H

struct loaded_bitmap
{
	int32_t Width;
	int32_t Height;
	int32_t Pitch;
	// NOTE: Memory is not the head of memory! it's actually offset so we can 
	// CONT: read it from the bottom left
	void* Memory;
};

#define APOCALYPSE_BITMAP_H
#endif