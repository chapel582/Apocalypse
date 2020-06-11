#ifndef APOCALYPSE_BITMAP_H

struct loaded_bitmap
{
	int32_t Width;
	int32_t Height;
	int32_t Pitch;
	void* Memory;
};

#define APOCALYPSE_BITMAP_H
#endif