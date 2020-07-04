#include "apocalypse_audio.h"
#include "apocalypse_platform.h"
#include "apocalypse_memory_arena.h"

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
	// NOTE: mix in float channels and then cast to int16_t so we don't get 
	// CONT: unnecessary clipping/wrap-around
	// TODO: PushArray is fast, but it might be faster to allocate this once
	// CONT: since we probably don't have to worry about SampleCount changing 
	// CONT: from frame to frame
	float* Channel0 = PushArray(FrameArena, SoundBuffer->SampleCount, float);
	float* Channel1 = PushArray(FrameArena, SoundBuffer->SampleCount, float);
	memset(Channel0, 0, sizeof(*Channel0) * SoundBuffer->SampleCount);
	memset(Channel1, 0, sizeof(*Channel1) * SoundBuffer->SampleCount);
	
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
			float Volume0 = PlayingSound->Volume[0];
			float Volume1 = PlayingSound->Volume[1];
			float* Dest0 = Channel0;
			float* Dest1 = Channel1;

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

			for(
				uint32_t SampleIndex = PlayingSound->SamplesPlayed;
				SampleIndex < (PlayingSound->SamplesPlayed + SamplesToMix);
				SampleIndex++
			)
			{
				// TODO: load stereo sound
				float SampleValue = LoadedSound->Samples[0][SampleIndex];
				*Dest0++ += Volume0 * SampleValue;
				*Dest1++ += Volume1 * SampleValue;
			}

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

	int16_t* SampleOut = SoundBuffer->Samples;
	float* Source0 = Channel0;
	float* Source1 = Channel1;
	for(
		uint32_t SampleIndex = 0;
		SampleIndex < SoundBuffer->SampleCount;
		SampleIndex++
	)
	{
		// NOTE: SampleOut writes left and right channels
		*SampleOut++ = (int16_t) (*Source0 + 0.5f);
		Source0++;
		*SampleOut++ = (int16_t) (*Source1 + 0.5f);
		Source1++;
	}
}