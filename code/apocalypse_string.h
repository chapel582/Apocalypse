#ifndef APOCALYPSE_STRING_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// TODO: move most of this code to the cpp file

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

string_appender MakeStringAppender(char* String, uint32_t MaxStringSize)
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

void LenAppendToString(
	string_appender* StringAppender, char* String, uint32_t Len
)
{
	memcpy(StringAppender->CopyTo, String, Len);
	
	StringAppender->CopyTo += Len;
	StringAppender->CharactersRemaining -= Len;
}

inline void TerminateString(string_appender* StringAppender)
{
	*StringAppender->CopyTo = 0;
}

struct flat_string_array_reader
{
	char* CurrentString;
	uint32_t BytesRemaining;
};

void InitFlatStringArrayReader(
	flat_string_array_reader* Reader, char* Buffer, uint32_t BufferSize
)
{
	*Reader = {};
	Reader->CurrentString = Buffer;
	Reader->BytesRemaining = BufferSize; 
}

char* GetNextString(flat_string_array_reader* Reader)
{
	// NOTE: returns NULL if next string cannot be found
	// NOTE: assumes memory past the last 0 is uninitialized 
	char* Checker = Reader->CurrentString;
	while((*Checker != 0) && (Reader->BytesRemaining > 0))
	{
		Checker++;
		Reader->BytesRemaining--;
	}
	while((*Checker == 0) && (Reader->BytesRemaining > 0))
	{
		Checker++;
		Reader->BytesRemaining--;
	}
	if(*Checker != 0)
	{
		Reader->CurrentString = Checker;
		return Checker;
	}
	else
	{
		return NULL;
	}
}

int32_t FindIndex(char* String, char Character, uint32_t BufferSize)
{
	char* LastChar = String;
	bool Found = false;
	uint32_t Index;
	for(Index = 0; Index < BufferSize; Index++)
	{
		if(*LastChar == Character)
		{
			Found = true;
			break;
		}
		else if(*LastChar == 0)
		{
			break;
		}
		LastChar++;
	}

	if(Found)
	{
		return Index;
	}
	else
	{
		return -1;
	}
}

bool StartsWith(char* String, char* Starter, uint32_t MaxSize)
{
	for(uint32_t Index = 0; Index < MaxSize; Index++)
	{
		if(*Starter != *String)
		{
			return false;
		}
		else if(*Starter == 0 || *String == 0)
		{
			break;
		}
		Starter++;
		String++;
	}

	return true;
}

bool StrCmp(char* String1, char* String2, uint32_t StopAt)
{
	for(uint32_t Index = 0; Index < StopAt; Index++)
	{
		if(*String1 != *String2)
		{
			return false;
		}
		else if(*String1 == 0 && *String2 == 0)
		{
			return true;
		}
		else if(*String1 == 0 || *String2 == 0)
		{
			return false;
		}
		String1++;
		String2++;
	}

	return true;
}

#define APOCALYPSE_STRING_H

#endif