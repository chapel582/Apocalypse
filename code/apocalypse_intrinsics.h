#ifndef APOCALYPSE_INTRINSICS_H

struct bit_scan_result
{
	uint32_t Index;
	bool Found;
};

inline bit_scan_result FindLeastSignificantSetBit(uint32_t Input)
{
	bit_scan_result Result = {};
#if COMPILER_MSVC
	Result.Found = _BitScanForward((unsigned long*) &Result.Index, Value);
#else
	uint32_t Mask = 0b1;
	int BitBeingChecked;
	for(BitBeingChecked = 0; BitBeingChecked < 32; BitBeingChecked++)
	{
		if((Input & Mask) > 0)
		{
			Result.Found = true;
			Result.Index = BitBeingChecked;
			break; 
		}
		Mask = Mask << 1;
	}
#endif
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

#define APOCALYPSE_INTRINSICS_H
#endif