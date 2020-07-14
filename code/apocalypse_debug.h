#ifndef APOCALYPSE_DEBUG_H

#include "apocalypse_platform.h"

#ifdef APOCALYPSE_INTERNAL
#define TIMED_BLOCK__(Number, ...) timed_block TimedBlock_##Number(__COUNTER__, __FILE__, __FUNCTION__, __LINE__, ## __VA_ARGS__);
#define TIMED_BLOCK_(Number, ...) TIMED_BLOCK__(Number, ## __VA_ARGS__);
#define TIMED_BLOCK(...) TIMED_BLOCK_(__LINE__, ## __VA_ARGS__);

struct debug_record
{
	uint64_t CycleCount;

	char* FileName;
	char* FunctionName;

	uint32_t LineNumber;
	uint32_t HitCount;
};

#define MAX_DEBUG_RECORDS 256
debug_record GlobalDebugRecords[MAX_DEBUG_RECORDS];

struct timed_block
{
	debug_record* Record;

	timed_block(
		int Id,
		char* FileName,
		char* FunctionName,
		int LineNumber,
		int HitCount = 1
	)
	{
		ASSERT(Id < MAX_DEBUG_RECORDS);
		Record = &GlobalDebugRecords[Id]; 
		Record->CycleCount -= PLATFORM_CYCLE_COUNT();
		Record->FileName = FileName;
		Record->FunctionName = FunctionName;
		Record->LineNumber = LineNumber;
		Record->HitCount += 1;
	}

	~timed_block()
	{
		Record->CycleCount += PLATFORM_CYCLE_COUNT();
	}
};

#else
#define TIMED_BLOCK(Id)

#endif // NOTE: APOCALYPSE_INTERNAL

#define APOCALYPSE_DEBUG_H
#endif