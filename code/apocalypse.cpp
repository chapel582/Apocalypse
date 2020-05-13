#include "apocalypse.h"

void GameUpdateAndRender(
	game_offscreen_buffer BackBuffer, int XOffset, int YOffset
)
{
	// NOTE: this is currently our render loop
	// CONT: it will be removed soon
	uint8_t* Row = (uint8_t*) BackBuffer.Memory;
	for(int Y = 0; Y < BackBuffer.Height; Y++)
	{
		uint32_t* Pixel = (uint32_t*) Row;
		for(int X = 0; X < BackBuffer.Width; X++)
		{
			// *Pixel = 0x000000;
			uint8_t* ColorChannel = (uint8_t*) Pixel;
			*ColorChannel++ = (uint8_t) (X + XOffset);
			*ColorChannel++ = (uint8_t) (Y + YOffset);
			*ColorChannel++ = 0;
			Pixel++;
		}
		Row += BackBuffer.Pitch;
	}
}