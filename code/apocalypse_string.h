#ifndef APOCALYPSE_STRING_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint32_t UInt32ToString(
	char* Buffer, uint32_t MaxBufferSize, uint32_t Input
)
{
	return snprintf(Buffer, MaxBufferSize, "%d", Input);
}

#define APOCALYPSE_STRING_H

#endif