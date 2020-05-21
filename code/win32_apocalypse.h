#ifndef WIN32_APOCALYPSE_H

#include <windows.h>
#include <stdint.h>

typedef enum
{
	WindowsSuccess,

	// InitDSound
	DSoundLibLoad,
	DSoundCreate,
	DSoundCooperative,
	DSoundPrimary,
	DSoundSecondary
} windows_result_code;

struct win32_sound_output
{
	int SamplesPerSecond;
	int SamplesInSecondaryBuffer;
	int NumSafetySamples;
	uint32_t RunningSampleIndex;
	int BytesPerSample;
	uint32_t BytesPerFrame;
	uint32_t SecondaryBufferSize;
	uint32_t SafetySize;
	int LatencySampleCount;
	// TODO: math gets simpler if we add a bytes per second field
};

struct win32_window_dimension
{
	uint32_t Width;
	uint32_t Height;
};

struct win32_offscreen_buffer
{
	int Width;
	int Height;
	int Pitch;
	int BytesPerPixel;
	void* Memory;
	BITMAPINFO Info;
};

struct win32_debug_time_marker
{
    uint32_t PlayCursor;
    uint32_t WriteCursor;
};

#define WIN32_APOCALYPSE_H
#endif