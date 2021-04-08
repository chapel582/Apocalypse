#include "apocalypse_platform.h"
#include "apocalypse_logging.h"

int32_t GetNextEmpty(platform_job_queue* JobQueue)
{
	ASSERT(JobQueue->EmptyEntriesCount > 0);
	if(JobQueue->EmptyEntriesCount == 0)
	{
		// TODO: handle errors
		// TODO: logging
		return -1;
	}
	uint32_t Result = JobQueue->EmptyEntries[JobQueue->EmptyEntriesStart];
	JobQueue->EmptyEntriesStart++;
	if(JobQueue->EmptyEntriesStart >= JOB_QUEUE_ENTRIES_COUNT)
	{
		JobQueue->EmptyEntriesStart = 0;	
	}
	JobQueue->EmptyEntriesCount--;
	return (int32_t) Result;
}

void AddEmpty(platform_job_queue* JobQueue, uint32_t NewEmpty)
{
	ASSERT(JobQueue->EmptyEntriesCount < JOB_QUEUE_ENTRIES_COUNT);
	JobQueue->EmptyEntries[JobQueue->EmptyEntriesCount] = NewEmpty;
	JobQueue->EmptyEntriesCount++;
	ASSERT(JobQueue->EmptyEntriesCount <= JOB_QUEUE_ENTRIES_COUNT);
}

inline uint32_t SafeTruncateUInt64(uint64_t Value)
{
	ASSERT(Value <= UINT32_MAX);
	return (uint32_t) Value;
}