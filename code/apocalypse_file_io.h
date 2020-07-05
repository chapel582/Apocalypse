#ifndef APOCALYPSE_FILE_IO_H

#include "apocalypse_platform.h"
#include "apocalypse_memory_arena.h"

platform_read_file_result ReadEntireFile(
	char* FileName, memory_arena* Arena, void** Result
)
{
	void* Data = NULL;
	size_t OldUsed = Arena->Used;
	
	platform_read_file_result ReadResult;
	uint32_t FileSize;
	ReadResult = PlatformGetFileSize(
		FileName, &FileSize
	);
	if(ReadResult != PlatformReadFileResult_Success)
	{
		goto error;
	}

	Data = PushSize(Arena, FileSize);

	ReadResult = PlatformReadFile(FileName, Data, FileSize);
	if(ReadResult != PlatformReadFileResult_Success)
	{
		goto error;
	}

	*Result = Data;
	goto end;

error:
	*Result = NULL;
	if(Data)
	{
		Arena->Used = OldUsed;
	}
end:
	return ReadResult;
}

// TODO: ReadEntireFile option with general allocator

#define APOCALYPSE_FILE_IO_H
#endif 