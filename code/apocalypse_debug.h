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

#define MAX_DEBUG_RECORDS_PER_THREAD 256

struct thread_debug_records
{
	uint32_t ThreadId;
	bool Initialized;
	debug_record Records[MAX_DEBUG_RECORDS_PER_THREAD];
};

// TODO: remove globals
thread_debug_records GlobalDebugRecords[MAX_THREAD_COUNT];

struct timed_block
{
	debug_record* Record;
	uint64_t StartCycleCount;

	timed_block(
		int Id,
		char* FileName,
		char* FunctionName,
		int LineNumber,
		int HitCount = 1
	)
	{
		ASSERT(Id < MAX_DEBUG_RECORDS_PER_THREAD);
		uint32_t ThreadId = GetThreadId();
		thread_debug_records* ThreadDebugRecords;
		int ThreadRecordIndex;
		for(
			ThreadRecordIndex = 0;
			ThreadRecordIndex < MAX_THREAD_COUNT;
			ThreadRecordIndex++
		)
		{
			ThreadDebugRecords = &GlobalDebugRecords[ThreadRecordIndex];
			if(ThreadId == ThreadDebugRecords->ThreadId)
			{
				break;
			}
			else if(!ThreadDebugRecords->Initialized)
			{
				ThreadDebugRecords->ThreadId = ThreadId;
				ThreadDebugRecords->Initialized = true;
				break;
			}
		}
		ASSERT(ThreadRecordIndex < MAX_THREAD_COUNT);

		Record = &ThreadDebugRecords->Records[Id]; 
		Record->FileName = FileName;
		Record->FunctionName = FunctionName;
		Record->LineNumber = LineNumber;
		AtomicAddUint32(&Record->HitCount, HitCount);
		StartCycleCount = PLATFORM_CYCLE_COUNT();
	}

	~timed_block()
	{
		Record->CycleCount += PLATFORM_CYCLE_COUNT() - StartCycleCount;
	}
};

#else
#define TIMED_BLOCK(Id)

#endif // NOTE: APOCALYPSE_INTERNAL

#define APOCALYPSE_DEBUG_H
#endif