#ifndef APOCALYPSE_INTRINSICS_H

struct bit_scan_result
{
	uint32_t Index;
	bool Found;
};

bit_scan_result FindLeastSignificantSetBit(uint32_t Input)
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

int32_t RoundFloat32ToInt32(float Input)
{
	return (int32_t) (Input + 0.5f);
}

float Floor(float Value)
{
	return ((float) ((int32_t) (Value)));
}

#define APOCALYPSE_INTRINSICS_H
#endif