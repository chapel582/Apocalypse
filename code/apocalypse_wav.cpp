#include "apocalypse_platform.h"

#include <stdint.h>

#pragma pack(push, 1)
struct WAVE_header
{
	uint32_t RIFFId;
	uint32_t WavSize;
	uint32_t WAVEId;
};

#define RIFF_CODE(a, b, c, d) (((uint32_t)(a) << 0) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))
enum
{
	WAVEChunkId_fmt = RIFF_CODE('f', 'm', 't', ' '),
	WAVEChunkId_data = RIFF_CODE('d', 'a', 't', 'a'),
	WAVEChunkId_RIFF = RIFF_CODE('R', 'I', 'F', 'F'),
	WAVEChunkId_WAVE = RIFF_CODE('W', 'A', 'V', 'E'),
};
struct WAVE_chunk
{
	uint32_t ID;
	uint32_t ChunkSize;
};

struct WAVE_fmt
{
	uint16_t wFormatTag;
	uint16_t nChannels;
	uint32_t nSamplesPerSec;
	uint32_t nAvgBytesPerSec;
	uint16_t nBlockAlign;
	uint16_t wBitsPerSample;
	uint16_t cbSize;
	uint16_t wValidBitsPerSample;
	uint32_t dwChannelMask;
	uint8_t SubFormat[16];
};
#pragma pack(pop)

struct riff_iterator
{
	uint8_t* At;
	uint8_t* Stop;
};

inline riff_iterator ParseChunkAt(void* At, void* Stop)
{
	riff_iterator Iter;

	Iter.At = (uint8_t*) At;
	Iter.Stop = (uint8_t*) Stop;

	return Iter;
}

inline riff_iterator NextChunk(riff_iterator Iter)
{
	WAVE_chunk* Chunk = (WAVE_chunk*) Iter.At;
	uint32_t TotalSize = (Chunk->ChunkSize + 1) & ~1;
	Iter.At += sizeof(WAVE_chunk) + TotalSize;

	return Iter;
}
			 
inline bool IsValid(riff_iterator Iter)
{    
	bool Result = (Iter.At < Iter.Stop);
	
	return Result;
}

inline void* GetChunkData(riff_iterator Iter)
{
	void* Result = (Iter.At + sizeof(WAVE_chunk));

	return Result;
}

inline uint32_t GetType(riff_iterator Iter)
{
	WAVE_chunk* Chunk = (WAVE_chunk*) Iter.At;
	uint32_t Result = Chunk->ID;

	return Result;
}

inline uint32_t GetChunkDataSize(riff_iterator Iter)
{
	WAVE_chunk* Chunk = (WAVE_chunk*) Iter.At;
	uint32_t Result = Chunk->ChunkSize;

	return Result;
}

loaded_wav LoadWav(char* FileName, memory_arena* Arena)
{
	// TODO: bulletproof this function

	loaded_wav Result = {};
	
	void* Contents;
	platform_read_file_result ReadResult = ReadEntireFile(
		FileName, Arena, &Contents
	);
	if(ReadResult != PlatformReadFileResult_Success)
	{
		goto error;
	}

	WAVE_header* Header = (WAVE_header*) Contents;
	ASSERT(Header->RIFFId == WAVEChunkId_RIFF);
	ASSERT(Header->WAVEId == WAVEChunkId_WAVE);

	uint32_t ChannelCount = 0;
	uint32_t SampleDataSize = 0;
	int16_t* SampleData = 0;
	for(
		riff_iterator Iter = (
			ParseChunkAt(
				Header + 1, ((uint8_t*) (Header + 1)) + Header->WavSize - 4
			)
		);
		IsValid(Iter);
		Iter = NextChunk(Iter)
	)
	{
		switch(GetType(Iter))
		{
			case(WAVEChunkId_fmt):
			{
				WAVE_fmt* fmt = (WAVE_fmt*) GetChunkData(Iter);
				ASSERT(fmt->wFormatTag == 1); // NOTE: Only support PCM
				ASSERT(fmt->nSamplesPerSec == 48000);
				ASSERT(fmt->wBitsPerSample == 16);
				ASSERT(
					fmt->nBlockAlign == (sizeof(int16_t) * fmt->nChannels)
				);
				ChannelCount = fmt->nChannels;
				break;
			}

			case(WAVEChunkId_data):
			{
				SampleData = (int16_t*) GetChunkData(Iter);
				SampleDataSize = GetChunkDataSize(Iter);
				break;
			}
		}
	}

	ASSERT(ChannelCount && SampleData);

	Result.ChannelCount = ChannelCount;
	Result.SampleCount = SampleDataSize / (ChannelCount * sizeof(int16_t));
	if(ChannelCount == 1)
	{
		Result.Samples[0] = SampleData;
		Result.Samples[1] = 0;
	}
	else if(ChannelCount == 2)
	{
		Result.Samples[0] = SampleData;
		Result.Samples[1] = SampleData + Result.SampleCount;


		
		for(
			uint32_t SampleIndex = 0;
			SampleIndex < Result.SampleCount;
			SampleIndex++
		)
		{
			int16_t Source = SampleData[2 * SampleIndex];
			SampleData[2 * SampleIndex] = SampleData[SampleIndex];
			SampleData[SampleIndex] = Source;
		}
	}
	else
	{
		ASSERT(!"Invalid channel count in WAV file");
	}

	// TODO: Load right channels!
	Result.ChannelCount = 1;

	goto end;

error:
end:
	return Result;
}