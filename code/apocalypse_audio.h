#ifndef APOCALYPSE_AUDIO_H
struct playing_sound;
struct playing_sound
{
	uint32_t SamplesPlayed;
	float Volume[2];
	wav_handle_e WavHandle;
	playing_sound* Next;
};

struct playing_sound_list
{
	playing_sound* PlayingSoundHead;
	playing_sound* FreePlayingSoundHead;
};

#define APOCALYPSE_AUDIO_H
#endif