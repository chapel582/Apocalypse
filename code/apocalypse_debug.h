#ifndef APOCALYPSE_DEBUG_MODULE

#include "apocalypse_platform.h"

#ifdef APOCALYPSE_INTERNAL
#define TIMED_BLOCK(Id) timed_block TimedBlock##Id(DebugCycleCounter_##Id); 

struct timed_block
{
	uint64_t StartCycleCount;
	uint32_t Id;

	timed_block(uint32_t IdInit)
	{
		Id = IdInit;
		StartCycleCount = PLATFORM_CYCLE_COUNT();
	}

	~timed_block()
	{
		END_TIMED_BLOCK_(Id, StartCycleCount);
	}
};

#else
#define TIMED_BLOCK(Id)

#endif // NOTE: APOCALYPSE_INTERNAL

#define APOCALYPSE_DEBUG_MODULE
#endif