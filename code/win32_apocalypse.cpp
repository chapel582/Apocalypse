/*
TODO: This is not a final platform layer
	- Fullscreen support
	- Threading
	- sleep
	- control cursor visibility
	- Hardware acceleration
	- Blit speed improvements
	- Raw input and support for multiple keyboards
*/

// NOTE: Apocalypse stuff
#include "apocalypse.cpp"
#include "apocalypse_platform.h"

// NOTE: C stuff
#include <stdio.h>
#include <stdint.h>
#include <math.h>

// NOTE: Windows stuff
#include <windows.h>
#include <Winuser.h>
#include <dsound.h>

// NOTE: Win32 Apocalypse stuff
#include "win32_apocalypse.h"

// TODO: this is a global for now
bool GlobalRunning = false;
LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer = NULL;
win32_offscreen_buffer GlobalBackBuffer = {};

// START SECTION: Performance counters
int64_t GlobalPerformanceFrequency = 0;
inline int64_t Win32GetWallClock(void)
{
	LARGE_INTEGER Result;
	// NOTE: QueryPerformanceCounter gets wall clock time
	QueryPerformanceCounter(&Result);
	return Result.QuadPart;
}

inline float Win32GetSecondsElapsed(int64_t Start, int64_t End)
{
	float Result;
	Result = (
		((float) (End - Start)) / 
		((float) GlobalPerformanceFrequency)
	);
	return Result;
}
// STOP SECTION: Performance counters

debug_read_file_result DEBUGPlatformReadEntireFile(
	thread_context* Thread, char* FileName
)
{
	debug_read_file_result Result = {};
	HANDLE FileHandle = INVALID_HANDLE_VALUE;

	FileHandle = CreateFileA(
		FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0
	);
	if(FileHandle == INVALID_HANDLE_VALUE)
	{
		goto error;
	}
	LARGE_INTEGER FileSize;
	if(!GetFileSizeEx(FileHandle, &FileSize))
	{
		// TODO: logging with getlasterror
		goto error;
	}

	uint32_t FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
	Result.Contents = VirtualAlloc(
		0, FileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE
	);
	if(Result.Contents == NULL)
	{
		// TODO: logging
		goto error;
	}

	DWORD BytesRead = 0;
	if(
		ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) &&
		(FileSize32 == BytesRead)
	)
	{
		Result.ContentsSize = FileSize32;
	}
	else
	{
		// TODO: logging
		goto error;
	}

	goto end;

error:
	if(FileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(FileHandle);	
	}
	if(Result.Contents != NULL)
	{
		VirtualFree(Result.Contents, 0, MEM_RELEASE);
		Result.Contents = NULL;
	}
end:
	return Result;
}

void DEBUGPlatformFreeFileMemory(thread_context* Thread, void* Memory)
{
	if(Memory)
	{
		VirtualFree(Memory, 0, MEM_RELEASE);		
	}
}

bool DEBUGPlatformWriteEntireFile(
	thread_context* Thread, char* FileName, void* Memory, uint32_t MemorySize
)
{
	bool Result = false;
	HANDLE FileHandle = INVALID_HANDLE_VALUE;
	FileHandle = CreateFileA(
		FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0
	);
	if(FileHandle == INVALID_HANDLE_VALUE)
	{
		// TODO: logging
		goto error;
	}

	DWORD BytesWritten;
	if(!WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0))
	{
		// TODO: logging
		goto error;
	}
	if(BytesWritten != MemorySize)
	{
		goto error;
	}
	
	Result = true;
	goto end;

error:
end:
	if(FileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(FileHandle);
	}
	return Result;
}

// NOTE: DLL stubs
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

// NOTE: Init DSound!
windows_result_code Win32InitDSound(
	HWND WindowHandle, int32_t SamplesPerSecond, int32_t BufferSize
)
{
	windows_result_code result = WindowsSuccess;
	HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
	LPDIRECTSOUND DirectSound = NULL;
	LPDIRECTSOUNDBUFFER PrimaryBuffer = NULL;
	
	if(!DSoundLibrary)
	{
		OutputDebugStringA("dsound failed to load\n");
		// TODO: consider if we should be able to play
		// CONT: without sound
		result = DSoundLibLoad;
		goto sounderror;
	}

	direct_sound_create* DirectSoundCreate = (
		(direct_sound_create*) GetProcAddress(
			DSoundLibrary, "DirectSoundCreate"
		)
	);
	if(
		!(
			DirectSoundCreate && 
			SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))
		)
	)
	{
		// TODO: consider if we should be able to play 
		// CONT: without sound
		OutputDebugStringA("DirectSoundCreate failed\n");
		result = DSoundCreate;
		goto sounderror;
	}

	WAVEFORMATEX WaveFormat = {};
	WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	WaveFormat.nChannels = 2;
	WaveFormat.nSamplesPerSec = SamplesPerSecond;
	WaveFormat.wBitsPerSample = 16;
	WaveFormat.nBlockAlign = (
		(WaveFormat.nChannels*WaveFormat.wBitsPerSample) / 8
	);
	WaveFormat.nAvgBytesPerSec = (
		WaveFormat.nSamplesPerSec*WaveFormat.nBlockAlign
	);
	WaveFormat.cbSize = 0;

	if(
		!SUCCEEDED(
			DirectSound->SetCooperativeLevel(
				WindowHandle, DSSCL_PRIORITY
			)
		)
	)
	{
		OutputDebugStringA("SetCooperativeLevel failed\n");
		result = DSoundCooperative;
		goto sounderror;
	}

	DSBUFFERDESC PrimaryBufferDescription = {};
	PrimaryBufferDescription.dwSize = (
		sizeof(PrimaryBufferDescription)
	);
	PrimaryBufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

	// NOTE: "Create" a primary buffer
	// TODO: DSBCAPS_GLOBALFOCUS?
	if(
		!SUCCEEDED(
			DirectSound->CreateSoundBuffer(
				&PrimaryBufferDescription, &PrimaryBuffer, 0
			)
		)
	)
	{
		OutputDebugStringA("Failed to create primary buffer\n");
		result = DSoundPrimary;
		goto sounderror;
	}

	 // TODO: DSBCAPS_GETCURRENTPOSITION2
	DSBUFFERDESC SecondaryBufferDescription = {};
	SecondaryBufferDescription.dwSize = (
		sizeof(SecondaryBufferDescription)
	);
	SecondaryBufferDescription.dwFlags = 0;
	SecondaryBufferDescription.dwBufferBytes = BufferSize;
	SecondaryBufferDescription.lpwfxFormat = &WaveFormat;
	if(
		!SUCCEEDED(
			DirectSound->CreateSoundBuffer(
				&SecondaryBufferDescription, &GlobalSecondaryBuffer, 0
			)
		)
	)
	{
		OutputDebugStringA(
			"Failed to create secondary buffer.\n"
		);
		result = DSoundSecondary;
		goto sounderror;
	}
	goto soundend;

sounderror:
	// TODO: do we have to do any cleanup?
soundend:
	return result;
}

void Win32DebugDrawVertical(
	win32_offscreen_buffer* BackBuffer, 
	int X,
	int Top, 
	int Bottom, 
	uint32_t Color
)
{
	uint8_t* Pixel = (
		((uint8_t*) BackBuffer->Memory) +
		X * BackBuffer->BytesPerPixel + 
		Top * BackBuffer->Pitch
	);
	for(int Y = Top; Y < Bottom; Y++)
	{
		*((uint32_t*) Pixel) = Color;
		Pixel += BackBuffer->Pitch;
	}
}

void Win32DrawSoundBufferMarker(
	win32_offscreen_buffer* BackBuffer,
	win32_sound_output* SoundOutput,
	float C, 
	int PadX,
	int Top,
	int Bottom,
	uint32_t Value,
	uint32_t Color
)
{
	ASSERT(Value < SoundOutput->SecondaryBufferSize);
	float XFloat = (C * ((float) Value));
	int X = PadX + ((int) XFloat);
	Win32DebugDrawVertical(BackBuffer, X, Top, Bottom, Color);
}

void Win32DebugSyncDisplay(
	win32_offscreen_buffer* BackBuffer,
	win32_debug_time_marker* Markers,
	int MarkerCount,
	win32_sound_output* SoundOutput
)
{
	int PadX = 16;
	int PadY = 16;
	int Top = PadY;
	int Bottom = BackBuffer->Height - PadY;

	float C = (
		(float) (BackBuffer->Width - 2 * PadX) / 
		(float) SoundOutput->SecondaryBufferSize
	);

	for(int MarkerIndex = 0; MarkerIndex < MarkerCount; MarkerIndex++)
	{
		win32_debug_time_marker* ThisMarker = &Markers[MarkerIndex];
		Win32DrawSoundBufferMarker(
			BackBuffer, 
			SoundOutput,
			C,
			PadX,
			Top,
			Bottom,
			ThisMarker->PlayCursor,
			0xFFFFFFFF
		);
		Win32DrawSoundBufferMarker(
			BackBuffer, 
			SoundOutput,
			C,
			PadX,
			Top,
			Bottom,
			ThisMarker->WriteCursor,
			0xFFFF0000
		);
	}
}

void Win32BufferToWindow(
	win32_offscreen_buffer* BackBuffer, 
	HDC DeviceContext
	// ,
	// int WindowWidth, 
	// int WindowHeight
)
{
	// TODO: aspect ratio correction
	// TODO: while prototyping, no stretching
	StretchDIBits(
		DeviceContext,
		0,
		0,
		BackBuffer->Width,
		BackBuffer->Height,
		0,
		0,
		BackBuffer->Width,
		BackBuffer->Height,
		BackBuffer->Memory,
		&BackBuffer->Info,
		DIB_RGB_COLORS,
		SRCCOPY
	);
}

win32_window_dimension Win32GetWindowDimension(HWND Window)
{
	RECT ClientRect = {};
	GetClientRect(Window, &ClientRect);
	win32_window_dimension Result = {};
	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;
	return Result;
}

void Win32WriteMouseEvent(
	game_mouse_events* MouseEvents,
	LPARAM LParam,
	mouse_event_type Type,
	user_event_index* UserEventIndex,
	uint32_t ScreenHeight
)
{
	game_mouse_event* MouseEvent = &MouseEvents->Events[MouseEvents->Length];
	MouseEvent->UserEventIndex = *UserEventIndex;
	*UserEventIndex += 1;
	MouseEvent->XPos = LParam & 0xFFFF;
	MouseEvent->YPos = ScreenHeight - ((LParam & 0xFFFF0000) >> 16);
	MouseEvent->Type = Type;
	MouseEvents->Length++;
	ASSERT(
		MouseEvents->Length < ARRAY_COUNT(MouseEvents->Events)
	);
}

void Win32ResetUserEvents(
	user_event_index* UserEventIndex, 
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents
)
{
	*UserEventIndex = 0;
	MouseEvents->Length = 0;
	KeyboardEvents->Length = 0;
}

LRESULT CALLBACK MainWindowCallback(
	HWND Window,
	UINT Message,
	WPARAM WParam,
	LPARAM LParam
)
{
	LRESULT Result = 0;
	switch(Message)
	{
		case(WM_SIZE):
		{
			break;
		}
		case(WM_ACTIVATEAPP):
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
			break;
		}
		case(WM_CLOSE):
		{
			GlobalRunning = false;
			break;
		}
		case(WM_DESTROY):
		{
			GlobalRunning = false;
			break;
		}
		case(WM_SYSKEYDOWN):
		case(WM_SYSKEYUP):
		case(WM_KEYDOWN):
		case(WM_KEYUP):
		{
			ASSERT(!"Keyboard event received in non-dispatch message");
			break;
		}
		case(WM_LBUTTONDOWN):
		case(WM_LBUTTONUP):
		case(WM_RBUTTONDOWN):
		case(WM_RBUTTONUP):
		case(WM_MOUSEMOVE):
		{
			ASSERT(!"Mouse event received in non-dispatch message");
			break;
		}
		case(WM_PAINT):
		{
			PAINTSTRUCT Paint = {};
			HDC DeviceContext = BeginPaint(Window, &Paint);
			// int WindowWidth = Paint.rcPaint.right - Paint.rcPaint.left;
			// int WindowHeight = Paint.rcPaint.bottom - Paint.rcPaint.top;
			// win32_window_dimension WindowDim = Win32GetWindowDimension(Window);
			
			Win32BufferToWindow(
				&GlobalBackBuffer,
				DeviceContext
				// WindowDim.Width,
				// WindowDim.Height
			);

			EndPaint(Window, &Paint);
			break;
		}
		default:
		{
			Result = DefWindowProc(Window, Message, WParam, LParam);
			break;
		}
	}

	return Result;
}

int CALLBACK WinMain(
	HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR CommandLine, 
	int ShowCode
)
{
	LARGE_INTEGER PerformanceFrequency;
	QueryPerformanceFrequency(&PerformanceFrequency);
	GlobalPerformanceFrequency = PerformanceFrequency.QuadPart;

	// NOTE: set the windows scheduler granularity to 1 ms so that our Sleep()
	// CONT: can be more granular
	UINT DesiredSchedulerMS = 1;
	bool SleepIsGranular = (
		timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR
	);

	GlobalBackBuffer = {};
	GlobalBackBuffer.BytesPerPixel = 4;

	WNDCLASS WindowClass = {};
	WindowClass.lpfnWndProc = MainWindowCallback;
	WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WindowClass.hInstance = Instance;
	WindowClass.lpszClassName = "ApocalypseWindowClass";

	// NOTE: if we ever have a variable framerate, we also have to update
	// CONT: sound output config
	win32_debug_time_marker DebugTimeMarkers[30] = {};
	int DebugTimeMarkerIndex = 0;

	if(RegisterClassA(&WindowClass))
	{
		HWND WindowHandle = CreateWindowExA(
			0,
			WindowClass.lpszClassName,
			"Apocalypses",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			Instance,
			0
		);
		
		// NOTE: get memory for backbuffer
		{
			win32_window_dimension Dimensions = Win32GetWindowDimension(
				WindowHandle
			);
			GlobalBackBuffer.Width = 960;
			GlobalBackBuffer.Height = 540;
			int BytesPerPixel = 4;
			GlobalBackBuffer.Pitch = GlobalBackBuffer.Width * BytesPerPixel;

			GlobalBackBuffer.Info.bmiHeader.biSize = (
				sizeof(GlobalBackBuffer.Info.bmiHeader)
			);
			GlobalBackBuffer.Info.bmiHeader.biWidth = GlobalBackBuffer.Width;
			GlobalBackBuffer.Info.bmiHeader.biHeight = GlobalBackBuffer.Height;
			GlobalBackBuffer.Info.bmiHeader.biPlanes = 1;
			GlobalBackBuffer.Info.bmiHeader.biBitCount = 32;
			GlobalBackBuffer.Info.bmiHeader.biCompression = BI_RGB;

			size_t BitmapMemorySize = (
				(GlobalBackBuffer.Width * GlobalBackBuffer.Height) * 
				BytesPerPixel
			);
			if(GlobalBackBuffer.Memory)
			{
				VirtualFree(GlobalBackBuffer.Memory, 0, MEM_RELEASE);
			}
			GlobalBackBuffer.Memory = VirtualAlloc(
				0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE
			);
		}

		if(WindowHandle)
		{
			// TODO: query this on Windows
			int MonitorRefreshHz = 60;
			{
				HDC RefreshDC = GetDC(WindowHandle);
				int Win32RefreshRate = GetDeviceCaps(RefreshDC, VREFRESH);
				ReleaseDC(WindowHandle, RefreshDC);
				if(Win32RefreshRate > 1)
				{
					MonitorRefreshHz = Win32RefreshRate;
				}
			}
			int GameUpdateHz = MonitorRefreshHz / 2;
			float TargetSecondsPerFrame = 1.0f / (float) GameUpdateHz;

#if APOCALYPSE_INTERNAL
			LPVOID BaseAddress = (LPVOID) TERABYTES(2);
#else
			LPVOID BaseAddress = 0;
#endif

			game_memory GameMemory = {};
			GameMemory.PermanentStorageSize = MEGABYTES(64);
			GameMemory.TransientStorageSize = GIGABYTES(4);
			GameMemory.PermanentStorage = VirtualAlloc(
				BaseAddress,
				(
					GameMemory.PermanentStorageSize + 
					GameMemory.TransientStorageSize
				),
				MEM_RESERVE | MEM_COMMIT,
				PAGE_READWRITE
			);
			GameMemory.TransientStorage = (
				((uint8_t*) GameMemory.PermanentStorage) + 
				GameMemory.PermanentStorageSize
			);
			if(!GameMemory.PermanentStorage || !GameMemory.TransientStorage)
			{
				// TODO: more logging
				OutputDebugStringA("Unable to allocate game memory\n");
				goto end;
			}

			user_event_index UserEventIndex = 0;
			game_mouse_events MouseEvents = {};
			game_keyboard_events KeyboardEvents = {};

			// NOTE: Since we specified CS_OWNDC, we can just grab this 
			// CONT: once and use it forever. No sharing
			HDC DeviceContext = GetDC(WindowHandle);
			windows_result_code result = Win32InitDSound(
				WindowHandle,
				48000,
				48000 * sizeof(int16_t) * 2
			);
			if(result != WindowsSuccess)
			{
				goto end;
			}
			bool SoundIsPlaying = false;

			// NOTE: for sound test
			win32_sound_output SoundOutput = {};
			SoundOutput.SamplesPerSecond = 48000;
			SoundOutput.RunningSampleIndex = 0;
			SoundOutput.BytesPerSample = sizeof(int16_t) * 2;
			SoundOutput.SamplesInSecondaryBuffer = SoundOutput.SamplesPerSecond; 
			SoundOutput.SecondaryBufferSize = (
				SoundOutput.SamplesInSecondaryBuffer * 
				SoundOutput.BytesPerSample
			);
			// TODO: figure out a more precise safety size to use
			// CONT: by calculating 
			SoundOutput.SafetySize = (uint32_t) (
				0.02f * 
				SoundOutput.SamplesPerSecond * 
				SoundOutput.BytesPerSample
			);
			SoundOutput.BytesPerFrame = (int) (
				TargetSecondsPerFrame *
				(float) SoundOutput.SamplesPerSecond *  
				(float) SoundOutput.BytesPerSample
			);
			uint32_t LastPlayCursor = 0; 
			// TODO: pool with bitmap alloc
			int16_t* SoundSamples = (int16_t*) VirtualAlloc(
				0,
				SoundOutput.SecondaryBufferSize,
				MEM_RESERVE | MEM_COMMIT,
				PAGE_READWRITE
			);

			int64_t FlipWallClock = Win32GetWallClock();
			GlobalRunning = true;
			while(GlobalRunning)
			{
				int64_t FrameStartCounter = Win32GetWallClock();
				// NOTE: rdtsc gets cycle counts instead of wall clock time
				uint64_t FrameStartCycle = __rdtsc();

				MSG Message = {};
				while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
				{
					if(Message.message == WM_QUIT)
					{
						GlobalRunning = false;
					}
					switch(Message.message)
					{
						case(WM_LBUTTONDOWN):
						{
							Win32WriteMouseEvent(
								&MouseEvents,
								Message.lParam,
								PrimaryDown,
								&UserEventIndex,
								GlobalBackBuffer.Height
							);
							break;
						}
						case(WM_LBUTTONUP):
						{
							Win32WriteMouseEvent(
								&MouseEvents,
								Message.lParam,
								PrimaryUp,
								&UserEventIndex,
								GlobalBackBuffer.Height
							);
							break;
						}
						case(WM_RBUTTONDOWN):
						{
							Win32WriteMouseEvent(
								&MouseEvents,
								Message.lParam,
								SecondaryDown,
								&UserEventIndex,
								GlobalBackBuffer.Height
							);
							break;
						}
						case(WM_RBUTTONUP):
						{
							Win32WriteMouseEvent(
								&MouseEvents,
								Message.lParam,
								SecondaryUp,
								&UserEventIndex,
								GlobalBackBuffer.Height
							);
							break;
						}
						case(WM_MOUSEMOVE):
						{
							Win32WriteMouseEvent(
								&MouseEvents,
								Message.lParam,
								MouseMove,
								&UserEventIndex,
								GlobalBackBuffer.Height
							);
							break;
						}
						case(WM_SYSKEYDOWN):
						case(WM_SYSKEYUP):
						case(WM_KEYDOWN):
						case(WM_KEYUP):
						{
							ASSERT(
								KeyboardEvents.Length < 
								ARRAY_COUNT(KeyboardEvents.Events)
							);
							game_keyboard_event* KeyboardEvent = (
								&KeyboardEvents.Events[KeyboardEvents.Length++]
							);

							KeyboardEvent->UserEventIndex = UserEventIndex++;
							ASSERT(((uint32_t) Message.wParam) <= 0xFF);
							KeyboardEvent->Code = (uint8_t) Message.wParam;
							KeyboardEvent->WasDown = (
								(Message.lParam & (1 << 30)) != 0
							);
							KeyboardEvent->IsDown = (
								(Message.lParam & (1 << 31)) == 0
							);
							
							break;
						}
						default:
						{
							TranslateMessage(&Message);
							DispatchMessageA(&Message);
							break;
						}
					}
				}				

				game_offscreen_buffer BackBuffer = {};
				BackBuffer.Memory = GlobalBackBuffer.Memory;
				BackBuffer.Width = GlobalBackBuffer.Width;
				BackBuffer.Height = GlobalBackBuffer.Height;
				BackBuffer.Pitch = GlobalBackBuffer.Pitch;
				BackBuffer.BytesPerPixel = GlobalBackBuffer.BytesPerPixel;
				thread_context Thread;
				GameUpdateAndRender(
					&Thread,
					&GameMemory,
					&BackBuffer,
					&MouseEvents,
					&KeyboardEvents,
					TargetSecondsPerFrame
				);
				Win32ResetUserEvents(
					&UserEventIndex, &MouseEvents, &KeyboardEvents
				);

				// SECTION START: Audio
				/* NOTE:
					On TargetCursor and BytesToWrite calculations

					we forecast to see where the play cursor will be during the 
					next frame boundary

					then see if the write cursor is before that by at least a 
					safety value. if it is, then the target fill position is 
					that frame boundary plus one frame 

					if the write cursor is after that safety margin, then just 
					write one frame's worth of audio   
				*/
				DWORD PlayCursor;
				DWORD WriteCursor;
				bool SoundIsValid = SUCCEEDED(
					GlobalSecondaryBuffer->GetCurrentPosition(
						&PlayCursor, &WriteCursor
					)
				);
				DWORD BytesToWrite = 0;
				DWORD ByteToLock;
				DWORD TargetCursor;
				if(SoundIsValid)
				{
					ByteToLock = (
						(
							SoundOutput.RunningSampleIndex * 
							SoundOutput.BytesPerSample
						) % 
						SoundOutput.SecondaryBufferSize
					);

					// NOTE: TargetCursor is not immediately used for 
					// CONT: determining BytesToWrite so it's necessary for it 
					// CONT: to be able to go past SecondaryBufferSize for 
					// CONT: consistent comparison with play and write cursors
					TargetCursor = PlayCursor + SoundOutput.BytesPerFrame;

					// NOTE: handle case when write cursor wrapped around
					DWORD TestWriteCursor;
					if(WriteCursor < PlayCursor)
					{
						TestWriteCursor = (
							SoundOutput.SecondaryBufferSize + WriteCursor
						);
					}
					else
					{
						TestWriteCursor = WriteCursor;
					}

					if(
						TargetCursor > 
						(TestWriteCursor + SoundOutput.SafetySize)
					)
					{
						TargetCursor += SoundOutput.BytesPerFrame;
					}
					else
					{
						TargetCursor = (
							WriteCursor + 
							SoundOutput.BytesPerFrame + 
							SoundOutput.SafetySize
						);
					}

					// NOTE: now we make sure TargetCursor is within buffer size
					TargetCursor %= SoundOutput.SecondaryBufferSize;

					// NOTE: this just handles the fact that our buffer is 
					// CONT: circular
					if(ByteToLock > TargetCursor)
					{
						// NOTE: write to end of buffer
						BytesToWrite = (
							SoundOutput.SecondaryBufferSize - ByteToLock
						);
						// NOTE: then write to target cursor
						BytesToWrite += TargetCursor;
					}
					else
					{
						// NOTE: write up to target cursor
						BytesToWrite = TargetCursor - ByteToLock;
					}
				}
				else
				{
					// TODO: Logging
					ByteToLock = 0;
					TargetCursor = 0;
					goto end;
				}

#if APOCALYPSE_INTERNAL
				{
					win32_debug_time_marker* Marker = (
						&DebugTimeMarkers[DebugTimeMarkerIndex++]
					);
					if(DebugTimeMarkerIndex >= ARRAY_COUNT(DebugTimeMarkers))
					{
						DebugTimeMarkerIndex = 0;
						LastPlayCursor = 
							DebugTimeMarkers[ARRAY_COUNT(DebugTimeMarkers) - 1].PlayCursor;
					}
					else
					{
						LastPlayCursor = 
							DebugTimeMarkers[DebugTimeMarkerIndex - 1].PlayCursor;	
					}
					Marker->PlayCursor = PlayCursor;
					Marker->WriteCursor = WriteCursor;
				}
#endif

				game_sound_output_buffer SoundBuffer = {};
				SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
				SoundBuffer.SampleCount = (
					BytesToWrite / SoundOutput.BytesPerSample
				);
				SoundBuffer.Samples = SoundSamples;
				GameFillSound(&GameMemory, &SoundBuffer);
				if(SoundIsValid)
				{
					// TODO: More strenuous test!
					// NOTE: there can be two regions because it's a circular 
					// CONT: buffer so we may need two pointers

					{
						char TextBuffer[256];
						sprintf_s(
							&TextBuffer[0],
							sizeof(TextBuffer),
							"LPC:%u BTL:%u TC:%u BTW %u - PC: %u WC: %u\n",
							LastPlayCursor,
							ByteToLock,
							TargetCursor,
							BytesToWrite,
							PlayCursor,
							WriteCursor
						);
						OutputDebugStringA(&TextBuffer[0]);
					}

					VOID* Region1;
					DWORD Region1Size;
					VOID* Region2;
					DWORD Region2Size;
					if(
						SUCCEEDED(
							GlobalSecondaryBuffer->Lock(
								ByteToLock,
								BytesToWrite,
								&Region1,
								&Region1Size,
								&Region2,
								&Region2Size,
								0
							)
						)
					)
					{
						// TODO: assert that Region1Size/Region2Size is valid

						// TODO: Collapse these two loops
						int16_t* SourceSample = SoundBuffer.Samples;
						DWORD Region1SampleCount = (
							Region1Size / SoundOutput.BytesPerSample
						);
						int16_t* SampleOut = (int16_t*) Region1;
						for(
							DWORD SampleIndex = 0;
							SampleIndex < Region1SampleCount;
							++SampleIndex
						)
						{
							// NOTE: SampleOut writes left and right channels
							*SampleOut++ = *SourceSample++;
							*SampleOut++ = *SourceSample++;
							SoundOutput.RunningSampleIndex++;
						}

						DWORD Region2SampleCount = (
							Region2Size / SoundOutput.BytesPerSample
						);
						SampleOut = (int16_t*) Region2;
						for(
							DWORD SampleIndex = 0;
							SampleIndex < Region2SampleCount;
							SampleIndex++
						)
						{
							// NOTE: SampleOut writes left and right channels
							*SampleOut++ = *SourceSample++;
							*SampleOut++ = *SourceSample++;
							SoundOutput.RunningSampleIndex++;
						}

						// NOTE: this is to avoid runningSampleIndex wrapping 
						// CONT: unexpectedly
						SoundOutput.RunningSampleIndex = (
							SoundOutput.RunningSampleIndex % 
							SoundOutput.SamplesInSecondaryBuffer
						);

						GlobalSecondaryBuffer->Unlock(
							Region1, Region1Size, Region2, Region2Size
						);
					}
				}

				if(!SoundIsPlaying)
				{
					GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
					SoundIsPlaying = true;
				}
				// SECTION STOP: Audio

				// SECTION START: Fixing frame rate to constant
				// TODO: more testing
				// TODO: have a way to log missing our framerate
				uint64_t WorkEndCounter = Win32GetWallClock();
				float WorkSeconds = Win32GetSecondsElapsed(
					FrameStartCounter, WorkEndCounter
				);
				float SecondsElapsedForFrame = WorkSeconds;
				if(SecondsElapsedForFrame < TargetSecondsPerFrame)
				{
					if(SleepIsGranular)
					{
						// NOTE: casting down so we don't sleep too long
						uint32_t SleepMs = (uint32_t) (
							1000.0f * (
								TargetSecondsPerFrame - SecondsElapsedForFrame
							)
						);
						if(SleepMs > 0)
						{
							Sleep(SleepMs - 1);
						}
					}

					while(SecondsElapsedForFrame < TargetSecondsPerFrame)
					{
						SecondsElapsedForFrame = Win32GetSecondsElapsed(
							FrameStartCounter, Win32GetWallClock()
						);
					}
				}
				else
				{
					// TODO: missed frame rate
					// TODO: logging
				}
				// SECTION STOP: Fixing frame rate to constant

				uint64_t FrameEndCycle = __rdtsc();
				int64_t FrameEndCounter = Win32GetWallClock();

				win32_window_dimension Dimensions = Win32GetWindowDimension(
					WindowHandle
				);
#if 0
				// NOTE: this is debug code
				Win32DebugSyncDisplay(
					&GlobalBackBuffer,
					DebugTimeMarkers,
					ARRAY_COUNT(DebugTimeMarkers),
					&SoundOutput
				);
#endif
				Win32BufferToWindow(
					&GlobalBackBuffer,
					DeviceContext
					// Dimensions.Width,
					// Dimensions.Height
				);
				FlipWallClock = Win32GetWallClock();
				

				// NOTE: Performance code 
				{
					float FrameMegaCycles = (
						(FrameEndCycle - FrameStartCycle) / 1000000.0f
					);
					float FrameSeconds = Win32GetSecondsElapsed(
						FrameStartCounter, FrameEndCounter
					);
					float Fps = 1.0f / FrameSeconds; 
					float FrameMs = 1000.0f * FrameSeconds;

					char FrameTimeBuffer[128];
					sprintf_s(
						&FrameTimeBuffer[0],
						sizeof(FrameTimeBuffer),
						"%.02ff/s, %.02f ms/f, %.02fMc/f\n",
						Fps, FrameMs, FrameMegaCycles
					);
					OutputDebugStringA(FrameTimeBuffer);
				}
				// NOTE: reset
				FrameStartCycle = FrameEndCycle;
				FrameStartCounter = FrameEndCounter;
			}
		}
		else
		{
			// TODO: Put this in a more unified logging location and get more 
			// CONT: info
			OutputDebugStringA("Failed to get window handle\n");
			goto end;
		}
	}
	else
	{
		// TODO: Put this in a more unified logging location and get more info
		OutputDebugStringA("Failed to register window class\n");
		goto end;
	}

end:
	return 0;
}