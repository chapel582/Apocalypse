#ifndef APOCALYPSE_BITMAP_H

struct loaded_bitmap
{
	int32_t Width;
	int32_t Height;
	uint32_t* Pixels;
};

#define APOCALYPSE_BITMAP_H
#endif