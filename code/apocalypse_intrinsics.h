#ifndef APOCALYPSE_INTRINSICS_H

struct bit_scan_result
{
	uint32_t Index;
	bool Found;
};

bit_scan_result FindLeastSignificantSetBit(uint32_t Input)
{
	bit_scan_result Result = {};
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
	return Result;
}

#define APOCALYPSE_INTRINSICS_H
#endif