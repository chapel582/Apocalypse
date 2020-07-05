#include "apocalypse_audio.h"
#include "apocalypse_platform.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_file_io.h"

void PlaySound(
	playing_sound_list* PlayingSoundList, wav_tag_e Tag, memory_arena* Arena
)
{
	if(!PlayingSoundList->FreePlayingSoundHead)
	{
		PlayingSoundList->FreePlayingSoundHead = PushStruct(
			Arena, playing_sound
		);
		PlayingSoundList->FreePlayingSoundHead->Next = NULL;
	}

	playing_sound* PlayingSound = PlayingSoundList->FreePlayingSoundHead;
	*PlayingSound = {};

	// NOTE: remove from free and add to playing list
	PlayingSoundList->FreePlayingSoundHead = PlayingSound->Next;
	PlayingSound->Next = PlayingSoundList->PlayingSoundHead;
	PlayingSoundList->PlayingSoundHead = PlayingSound;

	// NOTE: init data
	PlayingSound->SamplesPlayed = 0;
	PlayingSound->Volume[0] = 1.0f;
	PlayingSound->Volume[1] = 1.0f;
	PlayingSound->Tag = Tag;
}

void MixSounds(
	game_sound_output_buffer* SoundBuffer,
	memory_arena* FrameArena,
	assets* Assets,
	playing_sound_list* PlayingSoundList
)
{
	BEGIN_TIMED_BLOCK(MixAudio);

	// NOTE: mix in float channels and then cast to int16_t so we don't get 
	// CONT: unnecessary clipping/wrap-around
	// TODO: PushArray is fast, but it might be faster to allocate this once
	// CONT: since we probably don't have to worry about SampleCount changing 
	// CONT: from frame to frame
	uint32_t SampleCountOverFour = SoundBuffer->SampleCount / 4;
	
	__m128* Channel0 = PushArray(FrameArena, SampleCountOverFour, __m128, 16);
	__m128* Channel1 = PushArray(FrameArena, SampleCountOverFour, __m128, 16);

	// NOTE: clear mixer channels
	BEGIN_TIMED_BLOCK(MixAudio_Init);
	{
		// TODO: handle intrinsics on different platforms
		// TODO: maybe pull this out to intrinsics for fast zero setting?
		__m128 FourByZero = _mm_set1_ps(0.0f);
		__m128* Dest0 = Channel0;
		__m128* Dest1 = Channel1;
		for(
			uint32_t SampleIndex = 0;
			SampleIndex < SampleCountOverFour;
			SampleIndex++
		)
		{
			*Dest0++ = FourByZero;
			*Dest1++ = FourByZero;
		}
	}
	END_TIMED_BLOCK(MixAudio_Init);
	
	playing_sound* PrevPlayingSound = NULL;
	for(
		playing_sound* PlayingSound = PlayingSoundList->PlayingSoundHead;
		PlayingSound != NULL;
	)
	{
		playing_sound* NextPlayingSound = PlayingSound->Next;
		
		loaded_wav* LoadedSound = GetWav(Assets, PlayingSound->Tag);
		bool FinishedPlaying = false;
		if(LoadedSound != NULL)
		{
			uint32_t SamplesToMix;
			uint32_t SamplesRemainingInSound = (
				LoadedSound->SampleCount - PlayingSound->SamplesPlayed
			);
			if(SamplesRemainingInSound < SoundBuffer->SampleCount)
			{
				SamplesToMix = SamplesRemainingInSound;
			}
			else
			{
				SamplesToMix = SoundBuffer->SampleCount;
			}

			BEGIN_TIMED_BLOCK(MixAudioSample);
			__m128 Volume0 = _mm_set1_ps(PlayingSound->Volume[0]);
			__m128 Volume1 = _mm_set1_ps(PlayingSound->Volume[1]);
			__m128* Dest0 = Channel0;
			__m128* Dest1 = Channel1;

			uint32_t FinalSampleIndex = (
				PlayingSound->SamplesPlayed + SamplesToMix
			);
			uint32_t SampleIndex;
			for(
				SampleIndex = PlayingSound->SamplesPlayed;
				SampleIndex < FinalSampleIndex;
				SampleIndex += 4
			)
			{
				// TODO: load stereo sound
				__m128i WideSample0_16 = _mm_loadu_si128(
					(__m128i*) &LoadedSound->Samples[0][SampleIndex]
				);
				__m128i WideSample0_32 = _mm_cvtepi16_epi32(WideSample0_16);
				__m128 WideSample0 = _mm_cvtepi32_ps(WideSample0_32);
				__m128 WideDest0 = *Dest0;
				__m128 WideDest1 = *Dest1;

				WideDest0 = _mm_add_ps(
					WideDest0, _mm_mul_ps(Volume0, WideSample0)
				);
				WideDest1 = _mm_add_ps(
					WideDest1, _mm_mul_ps(Volume1, WideSample0)
				);
				*Dest0++ = WideDest0;
				*Dest1++ = WideDest1;
			}
			uint32_t Overshoot = SampleIndex - FinalSampleIndex;
			if(Overshoot)
			{
				float* OvershootDest0 = (float*) Dest0;
				float* OvershootDest1 = (float*) Dest1;
				for(
					uint32_t RemainderIndex = 0;
					RemainderIndex < Overshoot;
					RemainderIndex++
				)
				{
					// TODO: load stereo sound
					float SampleValue = (
						LoadedSound->Samples[0][SampleIndex - RemainderIndex]
					);
					*OvershootDest0-- = SampleValue;
					*OvershootDest1-- = SampleValue;
				}
			}
			END_TIMED_BLOCK_COUNTED(
				MixAudioSample, (PlayingSound->SamplesPlayed + SamplesToMix)
			);

			PlayingSound->SamplesPlayed += SamplesToMix;
			FinishedPlaying = (
				PlayingSound->SamplesPlayed == LoadedSound->SampleCount
			);
			if(FinishedPlaying)
			{
				if(PrevPlayingSound)
				{
					PrevPlayingSound->Next = PlayingSound->Next;
				}
				if(PlayingSoundList->PlayingSoundHead == PlayingSound)
				{
					PlayingSoundList->PlayingSoundHead = NextPlayingSound; 
				}
				PlayingSound->Next = PlayingSoundList->FreePlayingSoundHead;
				PlayingSoundList->FreePlayingSoundHead = PlayingSound;
			}
		}

		if(!FinishedPlaying)
		{
			// NOTE: Only update PrevPlaying if the element wasn't removed
			PrevPlayingSound = PlayingSound;
		}
		PlayingSound = NextPlayingSound;
	}

	BEGIN_TIMED_BLOCK(SetAudioSample);
	{ 
		//NOTE: 16-bit conversion
		__m128* Source0 = Channel0;
		__m128* Source1 = Channel1;

		__m128i* SampleOut = (__m128i*) SoundBuffer->Samples;
		for(uint32_t SampleIndex = 0;
			SampleIndex < SampleCountOverFour;
			SampleIndex++)
		{
			__m128 WideSource0 = *Source0++;
			__m128 WideSource1 = *Source1++;
			
			// NOTE: Convert floats to 32-bit integers
			__m128i Left = _mm_cvtps_epi32(WideSource0);
			__m128i Right = _mm_cvtps_epi32(WideSource1);
			
			// NOTE: pack/zip the low bytes
			__m128i Lr0 = _mm_unpacklo_epi32(Left, Right);
			// NOTE: pack/zip the higher bytes
			__m128i Lr1 = _mm_unpackhi_epi32(Left, Right);
			
			// NOTE: convert 32-bit integers to 16-bit integers, then pack side
			// CONT: by side
			__m128i S01 = _mm_packs_epi32(Lr0, Lr1);

			*SampleOut++ = S01;
		}
	}
	END_TIMED_BLOCK_COUNTED(SetAudioSample, SoundBuffer->SampleCount);

	END_TIMED_BLOCK(MixAudio);
}