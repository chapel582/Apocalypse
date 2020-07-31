#ifndef APOCALYPSE_INTRINSICS_H

struct bit_scan_result
{
	uint32_t Index;
	bool Found;
};

#if COMPILER_MSVC
inline bit_scan_result FindLeastSignificantSetBit(uint32_t Value)
{
	bit_scan_result Result = {};
	Result.Found = _BitScanForward((unsigned long*) &Result.Index, Value) != 0;
	return Result;
}

inline int32_t RoundFloat32ToInt32(float Input)
{
	return (int32_t) (Input + 0.5f);
}

inline float Floor(float Value)
{
	return ((float) ((int32_t) Value));
}

inline int32_t Int32Floor(float Value)
{
	return ((int32_t) Value);
}

inline float Ceil(float Value)
{
	return Floor(Value) + 1.0f;
}

inline int32_t Int32Ceil(float Value)
{
	return Int32Floor(Value) + 1;
}

inline uint64_t AtomicAddUint64(uint64_t volatile* Addend, uint64_t Value)
{
	uint64_t Result = _InterlockedExchangeAdd64((__int64*) Addend, Value);
	return Result;
}

inline uint32_t AtomicAddUint32(uint32_t volatile* Addend, uint32_t Value)
{
	uint32_t Result = _InterlockedExchangeAdd((long*) Addend, Value);
	return Result;
}

inline uint32_t GetThreadId(void)
{
	uint8_t* ThreadLocalStorage = (uint8_t*) __readgsqword(0x30);
	uint32_t ThreadId = *(uint32_t*)(ThreadLocalStorage + 0x48);
	return ThreadId;
} 

#else
inline bit_scan_result FindLeastSignificantSetBit(uint32_t Value)
{
	bit_scan_result Result = {};
	uint32_t Mask = 0b1;
	int BitBeingChecked;
	for(BitBeingChecked = 0; BitBeingChecked < 32; BitBeingChecked++)
	{
		if((Value & Mask) > 0)
		{
			Result.Found = true;
			Result.Index = BitBeingChecked;
			break; 
		}
		Mask = Mask << 1;
	}
	return Result;
}

#endif // NOTE: Compiler choice

#define APOCALYPSE_INTRINSICS_H
#endif