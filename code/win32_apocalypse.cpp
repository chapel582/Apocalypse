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

// NOTE: C stuff
#include <stdio.h>
#include <stdint.h>
#include <math.h>

// NOTE: Apocalypse stuff
#include "apocalypse.cpp"

// NOTE: Windows stuff
#include <windows.h>
#include <Winuser.h>
#include <dsound.h>

// NOTE: Win32 Apocalypse stuff
#include "win32_apocalypse.h"

#define ARRAY_COUNT(Array) (sizeof(Array) / sizeof(Array[0]))
// TODO: only assert on debug builds
#define ASSERT(Expression) if(!(Expression)) {*(int*) 0 = 0;}

#define Pi32 3.14159265359f

// TODO: this is a global for now
bool GlobalRunning = false;
LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer = NULL;

// NOTE: Performance counters
int64_t GlobalPerformanceFrequency = 0;
inline int64_t GetWallClock(void)
{
	LARGE_INTEGER Result;
	// NOTE: QueryPerformanceCounter gets wall clock time
	QueryPerformanceCounter(&Result);
	return Result.QuadPart;
}

inline float GetSecondsElapsed(int64_t Start, int64_t End)
{
	float Result;
	Result = (
		((float) (End - Start)) / 
		((float) GlobalPerformanceFrequency)
	);
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

struct win32_offscreen_buffer
{
	int Width;
	int Height;
	int Pitch;
	void* Memory;
	BITMAPINFO Info;
};

win32_offscreen_buffer GlobalBackBuffer = {};

void Win32BufferToWindow(HDC DeviceContext, int WindowWidth, int WindowHeight)
{
	// TODO: aspect ratio correction
	StretchDIBits(
		DeviceContext,
		0,
		0,
		WindowWidth,
		WindowHeight,
		0,
		0,
		GlobalBackBuffer.Width,
		GlobalBackBuffer.Height,
		GlobalBackBuffer.Memory,
		&GlobalBackBuffer.Info,
		DIB_RGB_COLORS,
		SRCCOPY
	);
}

struct win32_window_dimension
{
	int Width;
	int Height;
};

win32_window_dimension GetWindowDimension(HWND Window)
{
	RECT ClientRect = {};
	GetClientRect(Window, &ClientRect);
	win32_window_dimension Result = {};
	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;
	return Result;
}

apocalypse_mouse_events GlobalMouseEvents = {};

void Win32WriteMouseEvent(LPARAM LParam, mouse_event_type Type)
{
	apocalypse_mouse_event* MouseEvent = (
		&GlobalMouseEvents.Events[GlobalMouseEvents.Length]
	);
	MouseEvent->XPos = LParam & 0xFFFF;
	MouseEvent->YPos = (LParam & 0xFFFF0000) >> 16;
	MouseEvent->Type = Type;
	GlobalMouseEvents.Length++;
	ASSERT(
		GlobalMouseEvents.Length < ARRAY_COUNT(GlobalMouseEvents.Events)
	);
}

void Win32ResetMouseEvents()
{
	GlobalMouseEvents.Length = 0;
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
			uint32_t VKCode = (uint32_t) WParam;
			bool WasDown = ((LParam & (1 << 30))) != 0;
			bool IsDown = ((LParam & (1 << 31))) == 0;
			if(IsDown && (WasDown != IsDown))
			{
				if(VKCode == 'W')
				{
					OutputDebugStringA("Hit W\n");
				}
				else if(VKCode == 'A')
				{
					OutputDebugStringA("Hit A\n");
				}
				else if(VKCode == 'S')
				{
				}
				else if(VKCode == 'D')
				{
				}
				else if(VKCode == 'Q')
				{
				}
				else if(VKCode == 'E')
				{
				}
				else if(VKCode == VK_UP)
				{
				}
				else if(VKCode == VK_LEFT)
				{
				}
				else if(VKCode == VK_DOWN)
				{
				}
				else if(VKCode == VK_RIGHT)
				{
				}
				else if(VKCode == VK_ESCAPE)
				{
					OutputDebugStringA("ESCAPE: ");
					if(IsDown)
					{
						OutputDebugStringA("IsDown ");
					}
					if(WasDown)
					{
						OutputDebugStringA("WasDown");
					}
					OutputDebugStringA("\n");
				}
				else if(VKCode == VK_SPACE)
				{
				}
			}
			break;
		}
		case(WM_LBUTTONDOWN):
		{
			Win32WriteMouseEvent(LParam, PrimaryDown);
			break;
		}
		case(WM_LBUTTONUP):
		{
			Win32WriteMouseEvent(LParam, PrimaryUp);
			break;
		}
		case(WM_RBUTTONDOWN):
		{
			Win32WriteMouseEvent(LParam, SecondaryDown);
			break;
		}
		case(WM_RBUTTONUP):
		{
			Win32WriteMouseEvent(LParam, SecondaryUp);
			break;
		}
		case(WM_MOUSEMOVE):
		{
			Win32WriteMouseEvent(LParam, MouseMove);
			break;
		}
		case(WM_PAINT):
		{
			PAINTSTRUCT Paint = {};
			HDC DeviceContext = BeginPaint(Window, &Paint);
			int WindowWidth = Paint.rcPaint.right - Paint.rcPaint.left;
			int WindowHeight = Paint.rcPaint.bottom - Paint.rcPaint.top;
			
			Win32BufferToWindow(DeviceContext, WindowWidth, WindowHeight);

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

	GlobalBackBuffer = {};

	WNDCLASS WindowClass = {};
	WindowClass.lpfnWndProc = MainWindowCallback;
	WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WindowClass.hInstance = Instance;
	WindowClass.lpszClassName = "ApocalypseWindowClass";

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
			win32_window_dimension Dimensions = GetWindowDimension(WindowHandle);
			GlobalBackBuffer.Width = Dimensions.Width;
			GlobalBackBuffer.Height = Dimensions.Height;
			int BytesPerPixel = 4;
			GlobalBackBuffer.Pitch = GlobalBackBuffer.Width * BytesPerPixel;

			GlobalBackBuffer.Info.bmiHeader.biSize = (
				sizeof(GlobalBackBuffer.Info.bmiHeader)
			);
			GlobalBackBuffer.Info.bmiHeader.biWidth = (
				GlobalBackBuffer.Width
			);
			GlobalBackBuffer.Info.bmiHeader.biHeight = (
				-GlobalBackBuffer.Height
			);
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

			// NOTE: testing platform layer stuff
			mouse_event_type CurrentPrimaryState = PrimaryUp;
			int XOffset = 0;
			int YOffset = 0;

			// NOTE: for sound test
			int SamplesPerSecond = 48000;
			int ToneHz = 256;
			int WavePeriod = SamplesPerSecond / ToneHz;
			int16_t ToneVolume = 3000;
			uint32_t RunningSampleIndex = 0;
			int BytesPerSample = sizeof(int16_t) * 2;
			int SecondaryBufferSize = SamplesPerSecond*BytesPerSample;
			// NOTE: LatencySampleCount is how far ahead we need to write
			// NOTE: we write 1/15 of a second ahead
			int LatencySampleCount = SamplesPerSecond / 15;
			float SineAngle = 0.0f;

			GlobalRunning = true;
			while(GlobalRunning)
			{
				int64_t FrameStartCounter = GetWallClock();
				// NOTE: rdtsc gets cycle counts instead of wall clock time
				uint64_t FrameStartCycle = __rdtsc();

				MSG Message = {};
				while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
				{
					if(Message.message == WM_QUIT)
					{
						GlobalRunning = false;
					}

					TranslateMessage(&Message);
					DispatchMessageA(&Message);
				}

				// TODO: Have mouse code be platform independent
				for(
					int MouseEventIndex = 0;
					MouseEventIndex < GlobalMouseEvents.Length;
					MouseEventIndex++
				)
				{
					// TODO: remove last action wins stuff from 
					// NOTE: testing platform layer mouse stuff
					apocalypse_mouse_event* MouseEvent = (
						&GlobalMouseEvents.Events[MouseEventIndex]
					);

					if(
						MouseEvent->Type == PrimaryDown ||
						MouseEvent->Type == PrimaryUp
					)
					{
						CurrentPrimaryState = MouseEvent->Type;
					}

					if(CurrentPrimaryState == PrimaryDown)
					{
						XOffset = GlobalBackBuffer.Width - MouseEvent->XPos;
						YOffset = GlobalBackBuffer.Height - MouseEvent->YPos;
					}
				}
				Win32ResetMouseEvents();

				game_offscreen_buffer BackBuffer = {};
				BackBuffer.Memory = GlobalBackBuffer.Memory;
				BackBuffer.Width = GlobalBackBuffer.Width;
				BackBuffer.Height = GlobalBackBuffer.Height;
				BackBuffer.Pitch = GlobalBackBuffer.Pitch;
				GameUpdateAndRender(BackBuffer, XOffset, YOffset);

				// NOTE: DirectSound output test
				DWORD PlayCursor;
				DWORD WriteCursor;
				if(
					SUCCEEDED(
						GlobalSecondaryBuffer->GetCurrentPosition(
							&PlayCursor, &WriteCursor
						)
					)
				)
				{
					DWORD ByteToLock = (
						RunningSampleIndex * 
						BytesPerSample % 
						SecondaryBufferSize
					);
					// NOTE: we want to be a little bit ahead of where it is now
					DWORD TargetCursor = (
						(PlayCursor + (LatencySampleCount * BytesPerSample)) % 
						SecondaryBufferSize
					);
					DWORD BytesToWrite = 0;
					if(ByteToLock > TargetCursor)
					{
						// NOTE: write to end of buffer
						BytesToWrite = (SecondaryBufferSize - ByteToLock);
						BytesToWrite += TargetCursor;
					}
					else
					{
						// NOTE: write up to target cursor
						BytesToWrite = TargetCursor - ByteToLock;
					}

					// TODO: More strenuous test!
					// NOTE: there can be two regions because it's a circular 
					// CONT: buffer so we may need two pointers
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
						DWORD Region1SampleCount = Region1Size / BytesPerSample;
						int16_t* SampleOut = (int16_t*) Region1;
						for(
							DWORD SampleIndex = 0;
							SampleIndex < Region1SampleCount;
							++SampleIndex
						)
						{
							float SineValue = sinf(SineAngle);
							int16_t SampleValue = (
								(int16_t) (SineValue * ToneVolume)
							);
							SineAngle += (
								(2.0f * Pi32 * 1.0f) / ((float) WavePeriod)
							);

							// NOTE: SampleOut writes left and right channels
							*SampleOut++ = SampleValue;
							*SampleOut++ = SampleValue;
							RunningSampleIndex++;
						}

						DWORD Region2SampleCount = Region2Size / BytesPerSample;
						SampleOut = (int16_t*) Region2;
						for(
							DWORD SampleIndex = 0;
							SampleIndex < Region2SampleCount;
							SampleIndex++
						)
						{
							float SineValue = sinf(SineAngle);
							int16_t SampleValue = (
								(int16_t) (SineValue * ToneVolume)
							);
							SineAngle += (
								(2.0f * Pi32 * 1.0f) / ((float) WavePeriod)
							);

							// NOTE: SampleOut writes left and right channels
							*SampleOut++ = SampleValue;
							*SampleOut++ = SampleValue;
							RunningSampleIndex++;
						}

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

				win32_window_dimension Dimensions = GetWindowDimension(
					WindowHandle
				);
				Win32BufferToWindow(
					DeviceContext, Dimensions.Width, Dimensions.Height
				);
				

				// NOTE: Performance code 
				uint64_t FrameEndCycle = __rdtsc();
				int64_t FrameEndCounter = GetWallClock();

				float FrameMegaCycles = (
					(FrameEndCycle - FrameStartCycle) / 1000000.0f
				);
				float FrameSeconds = GetSecondsElapsed(
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