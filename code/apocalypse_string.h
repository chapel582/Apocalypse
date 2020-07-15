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

char* FindFirst(char* String, uint32_t MaxStringSize, char SearchFor)
{
	char* Check = String;
	for(uint32_t Index = 0; Index < MaxStringSize; Index++)
	{
		if(*Check == SearchFor)
		{
			return Check;
		}
		else if(*Check == 0)
		{
			break;
		}
		Check++;
	}

	return NULL;
}

char* FindLast(char* String, uint32_t MaxStringSize, char SearchFor)
{
	char* Result = NULL;
	char* Check = String;
	for(uint32_t Index = 0; Index < MaxStringSize; Index++)
	{
		if(*Check == SearchFor)
		{
			Result = Check;
		}
		else if(*Check == 0)
		{
			break;
		}
		Check++;
	}

	return Result;
}

#define APOCALYPSE_STRING_H

#endif