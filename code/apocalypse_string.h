#ifndef APOCALYPSE_STRING_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

char Lower(char Char)
{
	if(Char >= 0x41 && Char <= 0x5A)
	{
		return Char + 0x20;
	}
	else
	{
		return Char;
	}
}

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

struct string_appender
{
	uint32_t MaxStringSize;
	uint32_t CharactersRemaining;
	char* String;
	char* CopyTo;
};

inline string_appender MakeStringAppender(char* String, uint32_t MaxStringSize)
{
	string_appender Result = {};
	Result.String = String;
	Result.CopyTo = Result.String; 
	Result.MaxStringSize = MaxStringSize;
	Result.CharactersRemaining = MaxStringSize;
	return Result;
}

void AppendToString(string_appender* StringAppender, char* FormatString, ...)
{
	va_list ArgPtr;
	va_start(ArgPtr, FormatString);
	int WrittenBytes = vsnprintf(
		StringAppender->CopyTo,
		StringAppender->CharactersRemaining,
		FormatString,
		ArgPtr
	);
	va_end(ArgPtr);
	
	StringAppender->CopyTo += WrittenBytes;
	StringAppender->CharactersRemaining -= WrittenBytes;
}

inline void TerminateString(string_appender* StringAppender)
{
	*StringAppender->CopyTo = 0;
}

#define APOCALYPSE_STRING_H

#endif