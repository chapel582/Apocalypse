#include "apocalypse_sequence_alignment.h"

#define DEFAULT_GAP_COST 1.0f
#define DEFAULT_MISMATCH_COST 1.0f
#define DEFAULT_CASE_MISMATCH_COST (DEFAULT_MISMATCH_COST / 4.0f)

float* AllocScoreMemory(
	memory_arena* Arena,
	uint32_t MaxString1Size, 
	uint32_t MaxString2Size
)
{
	return PushArray(Arena, MaxString1Size * MaxString2Size, float);
}

inline float* ScorePtr(
	float* ScoreMemory, uint32_t NumCols, uint32_t Row, uint32_t Col
)
{
	return ScoreMemory + Row * NumCols + Col;
}

inline void SetScore(
	float* ScoreMemory,
	uint32_t NumCols,
	uint32_t Row,
	uint32_t Col,
	float Value
)
{
	*ScorePtr(ScoreMemory, NumCols, Row, Col) = Value;
}

inline float GetScore(
	float* ScoreMemory, uint32_t NumCols, uint32_t Row, uint32_t Col
)
{
	return *ScorePtr(ScoreMemory, NumCols, Row, Col);
}

float SequenceAlignmentScore(
	char* String1,
	uint32_t String1Size,
	char* String2,
	uint32_t String2Size,
	float* ScoreMemory
)
{
	uint32_t String2Len = 0;
	char* CharPtr = String2;
	for(uint32_t Col = 0; Col < String2Size; Col++)
	{
		if(*CharPtr == 0)
		{
			break;
		}

		ScoreMemory[Col] = Col * DEFAULT_GAP_COST;
		CharPtr++;
		String2Len++;
	}

	uint32_t String1Len = 0;
	CharPtr = String1;
	for(uint32_t Row = 0; Row < String1Size; Row++)
	{
		if(*CharPtr == 0)
		{
			break;
		}

		float* WriteTo = ScoreMemory + Row * String2Len;
		*WriteTo = Row * DEFAULT_GAP_COST;
		CharPtr++;
		String1Len++;
	}

	/* NOTE: 
		the + 1 in the loop-termination condition is there because the 0th index
		is actually padding for the dynamic programming algorithm to not lookup 
		into invalid data. Therefore, the optimal alignment for M(1, 1) 
		corresponds to the best alignment for String1[0] and String2[0]
	*/
	for(uint32_t Col = 1; Col < String2Len + 1; Col++)
	{
		for(uint32_t Row = 1; Row < String1Len + 1; Row++)
		{
			float Options[3];
			float Mismatch;
			if(String1[Row] == String2[Col])
			{
				Mismatch = 0.0f;
			}
			else if(Lower(String1[Row]) == Lower(String2[Row]))
			{
				Mismatch = DEFAULT_CASE_MISMATCH_COST;
			}
			else
			{
				Mismatch = DEFAULT_MISMATCH_COST;
			}
			Options[0] = (
				Mismatch + GetScore(ScoreMemory, String2Len, Row - 1, Col - 1)
			);
			Options[1] = (
				DEFAULT_GAP_COST + 
				GetScore(ScoreMemory, String2Len, Row - 1, Col)
			);
			Options[2] = (
				DEFAULT_GAP_COST + 
				GetScore(ScoreMemory, String2Len, Row, Col - 1)
			);

			float Min = Options[0];
			for(
				uint32_t OptionIndex = 1;
				OptionIndex < ARRAY_COUNT(Options);
				OptionIndex++
			)
			{
				if(Options[OptionIndex] < Min)
				{
					Min = Options[OptionIndex];
				}
			}

			SetScore(ScoreMemory, String2Len, Row, Col, Min);
		}
	}

	return GetScore(ScoreMemory, String2Len, String1Len, String2Len);
}