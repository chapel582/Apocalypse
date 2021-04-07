/*
TODO: This is not a final platform layer
	- Fullscreen support
	- Non-job threading
	- sleep
	- control cursor visibility
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

// NOTE: OpenGL stuff
#include "apocalypse_opengl.cpp"

// NOTE: Win32 Apocalypse stuff
#include "win32_apocalypse.h"

// TODO: this is a global for now
bool GlobalRunning = false;
HWND GlobalWindowHandle = 0;
uint32_t GlobalWindowWidth = 0;
uint32_t GlobalWindowHeight = 0;
HGLRC GlobalOpenGlrc;
LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer = NULL;
GLuint GlobalBlitTextureHandle;

#define WINDOW_STYLE (WS_OVERLAPPEDWINDOW | WS_VISIBLE)

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

platform_read_file_result Win32GetFileSize(
	HANDLE FileHandle, uint32_t* FileSize
)
{
	platform_read_file_result Result = PlatformReadFileResult_Failure;

	LARGE_INTEGER LargeFileSize;
	if(!GetFileSizeEx(FileHandle, &LargeFileSize))
	{
		// TODO: logging with getlasterror
		goto error;
	}
	
	*FileSize = SafeTruncateUInt64(LargeFileSize.QuadPart);
	Result = PlatformReadFileResult_Success;
	goto end;

error:
end:
	return Result;
}

platform_read_file_result PlatformGetFileSize(
	char* FileName, uint32_t* FileSize
)
{
	platform_read_file_result Result = PlatformReadFileResult_Failure;
	HANDLE FileHandle = INVALID_HANDLE_VALUE;

	FileHandle = CreateFileA(
		FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0
	);

	Result = Win32GetFileSize(FileHandle, FileSize);
	if(Result != PlatformReadFileResult_Success)
	{
		goto error;
	}

	goto end;

error:
end:
	if(FileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(FileHandle);
	}
	return Result;
}

platform_read_file_result PlatformReadFile(
	char* FileName, void* Contents, uint32_t BytesToRead
)
{
	// TODO: Probably need to add an offset and seek  option and a way to 
	// CONT: maintain the handle
	platform_read_file_result Result = PlatformReadFileResult_Failure;
	HANDLE FileHandle = INVALID_HANDLE_VALUE;

	FileHandle = CreateFileA(
		FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0
	);
	if(FileHandle == INVALID_HANDLE_VALUE)
	{
		goto error;
	}
	
	DWORD BytesRead = 0;
	if(
		ReadFile(FileHandle, Contents, BytesToRead, &BytesRead, 0) &&
		(BytesToRead == BytesRead)
	)
	{
		Result = PlatformReadFileResult_Success;
	}
	else
	{
		// TODO: logging
		goto error;
	}

	goto end;

error:
end:
	if(FileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(FileHandle);	
	}
	return Result;
}

bool PlatformWriteEntireFile(
	char* FileName, void* Memory, uint32_t MemorySize
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

	if(Memory == NULL || MemorySize == 0)
	{
		goto end;
	}
	
	DWORD BytesWritten = 0;
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

bool PlatformAppendToFile(
	char* FileName, void* Memory, uint32_t MemorySize
)
{
	// NOTE: MemorySize should be the amount of memory to write (not the size)
	// CONT: of the buffer
	bool Result = false;
	HANDLE FileHandle = INVALID_HANDLE_VALUE;
	FileHandle = CreateFileA(
		FileName, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0
	);
	if(FileHandle == INVALID_HANDLE_VALUE)
	{
		// TODO: logging
		goto error;
	}

	uint32_t FileSize = 0;
	Win32GetFileSize(FileHandle, &FileSize);
	SetFilePointer(FileHandle, FileSize, 0, FILE_BEGIN);

	DWORD BytesWritten = 0;
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

// TODO: error codes and handling
void PlatformMakeDirectory(char* Path)
{
	LPSECURITY_ATTRIBUTES SecurityAttributes = NULL;
	bool Result = CreateDirectoryA(Path, SecurityAttributes);
	if(!Result)
	{
		if(GetLastError() == ERROR_ALREADY_EXISTS)
		{
			return;
		}
		else
		{
			// TODO: error handling
		}
	}
	// TODO: handle success
}

void PlatformFindAllFiles(
	char* FilePattern, char* FileNames, uint32_t FileNamesSize 
)
{
	WIN32_FIND_DATA FindData;
	HANDLE FindHandle = FindFirstFileA(FilePattern, &FindData);
	if(INVALID_HANDLE_VALUE == FindHandle)
	{
		// TODO: return error code
		goto end;
	}

	uint32_t BytesLeftInBuffer = FileNamesSize;
	char* CopyTo = FileNames;
	do
	{
		uint32_t FileNameLen = (uint32_t) strlen(FindData.cFileName);
		memcpy(CopyTo, FindData.cFileName, FileNameLen);
		BytesLeftInBuffer -= FileNameLen;
		CopyTo += FileNameLen + 1;
	} while(FindNextFileA(FindHandle, &FindData) != 0);

	FindClose(FindHandle);
	goto end;

end:
	return;
}

void PlatformDeleteFile(char* File)
{
	// TODO: error handling
	DeleteFileA(File);
}

platform_socket_result PlatformCreateListen(platform_socket* ListenSocket)
{
	platform_socket_result Result = PlatformSocketResult_Success;
	
	addrinfo* AddrInfo = NULL;
	SOCKET Win32ListenSocket = INVALID_SOCKET;

	WSADATA WsaData = {};
	int WsaResult = WSAStartup(MAKEWORD(2, 2), &WsaData);
	if(WsaResult != 0)
	{
		// TODO: logging
		// TODO: see if this should be done during application startup?
		Result = PlatformSocketResult_SocketModuleInitFail;
		goto error;
	}

	addrinfo Hints = {};
	Hints.ai_family = AF_INET6;
	Hints.ai_socktype = SOCK_STREAM;
	Hints.ai_protocol = IPPROTO_TCP;
	Hints.ai_flags = AI_PASSIVE;

	int AddrInfoResult = getaddrinfo(
		NULL, DEFAULT_PORT, &Hints, &AddrInfo
	);
	if(AddrInfoResult != 0)
	{
		Result = PlatformSocketResult_AddrInfoFail;
		// TODO: logging
		goto error;
	}

	Win32ListenSocket = socket(
		AddrInfo->ai_family, AddrInfo->ai_socktype, AddrInfo->ai_protocol
	);
	if(Win32ListenSocket == INVALID_SOCKET)
	{
		Result = PlatformSocketResult_MakeSocketFail;
		// TODO: logging
		goto error;
	}
	u_long ListenSocketMode = 1;  // NOTE: 1 to enable non-blocking socket
	ioctlsocket(Win32ListenSocket, FIONBIO, &ListenSocketMode);
	
	int Ipv6Only = 0;
	int SetSockOptResult = setsockopt(
		Win32ListenSocket,
		IPPROTO_IPV6,
		IPV6_V6ONLY,
		(char*) &Ipv6Only,
		sizeof(Ipv6Only)
	);
	if(SetSockOptResult == SOCKET_ERROR)
	{
		Result = PlatformSocketResult_EnableIpv6Fail;
		goto error;
	}

	int BindResult = bind(
		Win32ListenSocket, AddrInfo->ai_addr, (int) AddrInfo->ai_addrlen
	);
	if(BindResult == SOCKET_ERROR)
	{
		// TODO: logging
		Result = PlatformSocketResult_BindSocketFail;
		goto error;
	}

	// TODO: we might need to pull this out to a function that doesn't block?
	if(listen(Win32ListenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		// TODO: logging
		Result = PlatformSocketResult_ListenSocketFail;
		goto error;
	}

	ListenSocket->Socket = Win32ListenSocket;
	ListenSocket->IsValid = true;
	goto end;

error:
	if(Win32ListenSocket != INVALID_SOCKET)
	{
		closesocket(Win32ListenSocket);
	}
	if(WsaResult != 0)
	{
		WSACleanup();
	}

end:
	if(AddrInfo != NULL)
	{
		freeaddrinfo(AddrInfo);
	}
	return Result;
}

platform_socket_result PlatformAcceptConnection(
	platform_socket* ListenSocket, platform_socket* ClientSocketResult
)
{
	// TODO: should we stop listening after this happens?

	SOCKET Win32ClientSocket = INVALID_SOCKET;
	platform_socket_result Result = PlatformSocketResult_Success;

	Win32ClientSocket = accept(ListenSocket->Socket, NULL, NULL);
	if(Win32ClientSocket == INVALID_SOCKET)
	{
		int LastError = WSAGetLastError();
		if(LastError == WSAEWOULDBLOCK)
		{
			Result = PlatformSocketResult_NoPendingConnections;
		}
		else
		{
			// TODO: logging
			Result = PlatformSocketResult_AcceptFail;
		}
		goto error;
	}

	ClientSocketResult->Socket = Win32ClientSocket;

	goto end;

error:
	if(Win32ClientSocket != INVALID_SOCKET)
	{
		closesocket(Win32ClientSocket);
	}
end:
	return Result;
}

void PlatformCloseSocket(platform_socket* Socket)
{
	int ShutdownResult = shutdown(Socket->Socket, SD_SEND);
	if(ShutdownResult == SOCKET_ERROR)
	{
		// TODO: logging
	}

	closesocket(Socket->Socket);
	Socket->IsValid = false;
}

void PlatformServerDisconnect(
	platform_socket* ListenSocket, platform_socket* ClientSocket
)
{
	int ShutdownResult = shutdown(ClientSocket->Socket, SD_SEND);
	if(ShutdownResult == SOCKET_ERROR)
	{
		// TODO: logging
	}

	PlatformCloseSocket(ListenSocket);
	PlatformCloseSocket(ClientSocket);
	WSACleanup();
}

platform_socket_result PlatformCreateClient(
	char* ServerIp, platform_socket* ConnectSocket
)
{
	platform_socket_result Result = PlatformSocketResult_Success;
	
	addrinfo* AddrInfo = NULL;
	SOCKET Win32ConnectSocket = INVALID_SOCKET;

	WSADATA WsaData = {};
	int WsaResult = WSAStartup(MAKEWORD(2, 2), &WsaData);
	if(WsaResult != 0)
	{
		// TODO: logging
		// TODO: see if this should be done during application startup?
		Result = PlatformSocketResult_SocketModuleInitFail;
		goto error;
	}

	bool IsIpv6Address = strlen(ServerIp) > 16;

	addrinfo Hints = {};
	if(IsIpv6Address)
	{
		Hints.ai_family = AF_INET6;
	}
	else
	{
		Hints.ai_family = AF_INET;
	}
	Hints.ai_socktype = SOCK_STREAM;
	Hints.ai_protocol = IPPROTO_TCP;

	int AddrInfoResult = getaddrinfo(
		ServerIp, DEFAULT_PORT, &Hints, &AddrInfo
	);
	if(AddrInfoResult != 0)
	{
		Result = PlatformSocketResult_AddrInfoFail;
		// TODO: logging
		goto error;
	}

	Win32ConnectSocket = socket(
		AddrInfo->ai_family, AddrInfo->ai_socktype, AddrInfo->ai_protocol
	);
	if(Win32ConnectSocket == INVALID_SOCKET)
	{
		// TODO: logging
		Result = PlatformSocketResult_MakeSocketFail;
		goto error;
	}

	if(IsIpv6Address)
	{
		int Ipv6Only = 0;
		int SetSockOptResult = setsockopt(
			Win32ConnectSocket,
			IPPROTO_IPV6,
			IPV6_V6ONLY,
			(char*) &Ipv6Only,
			sizeof(Ipv6Only)
		);
		if(SetSockOptResult == SOCKET_ERROR)
		{
			Result = PlatformSocketResult_EnableIpv6Fail;
			goto error;
		}
	}

	int ConnectResult = connect(
		Win32ConnectSocket, AddrInfo->ai_addr, (int) AddrInfo->ai_addrlen
	);
	if(ConnectResult == SOCKET_ERROR)
	{
		// TODO: logging
		Result = PlatformSocketResult_ConnectSocketFail;
		goto error;
	}

	ConnectSocket->Socket = Win32ConnectSocket;
	ConnectSocket->IsValid = true;
	goto end;

error:
	if(Win32ConnectSocket != INVALID_SOCKET)
	{
		closesocket(Win32ConnectSocket);
	}
	if(WsaResult != 0)
	{
		WSACleanup();
	}
end:
	if(AddrInfo != NULL)
	{
		freeaddrinfo(AddrInfo);
	}
	return Result;
}

void PlatformClientDisconnect(platform_socket* ConnectSocket)
{
	PlatformCloseSocket(ConnectSocket);
	WSACleanup();
}

platform_socket_send_result PlatformSocketSend(
	platform_socket* Socket, void* Buffer, uint32_t DataSize
)
{
	platform_socket_send_result Result = PlatformSocketSendResult_Success;
	int SendResult = send(
		Socket->Socket, (const char*) Buffer, DataSize, 0
	);
	if(SendResult == SOCKET_ERROR)
	{
		int SpecificError = WSAGetLastError();
		if(SpecificError == WSAECONNRESET) // TODO: might need to check for WSAENETRESET as well 
		{
			Result = PlatformSocketSendResult_PeerReset;
		}
		else
		{
			Result = PlatformSocketSendResult_Error;
		}
	}

	return Result;
}

platform_socket_read_result PlatformSocketRead(
	platform_socket* Socket,
	void* Buffer,
	uint32_t BufferSize,
	uint32_t* TotalBytesRead
)
{
	platform_socket_read_result ReadResult = PlatformSocketReadResult_Success;
	int RemainingLength = BufferSize;
	int RecvResult = 0;
	int BytesRead = 0;
	uint8_t* WriteTo = (uint8_t*) Buffer;
	do
	{
		uint32_t BytesAvailable = 0;
		int PeekResult = ioctlsocket(
			Socket->Socket, FIONREAD, (u_long*) &BytesAvailable
		);
		if(PeekResult == SOCKET_ERROR)
		{
			break;
		}
		if(BytesAvailable == 0)
		{
			int SendResult = send(
				Socket->Socket, NULL, 0, 0
			);
			if(SendResult == SOCKET_ERROR)
			{
				int SpecificError = WSAGetLastError();
				if(SpecificError == WSAECONNRESET) // TODO: might need to check for WSAENETRESET as well 
				{
					ReadResult = PlatformSocketReadResult_PeerReset;
				}
			}
			break;
		}

		RecvResult = recv(Socket->Socket, (char*) WriteTo, RemainingLength, 0);
		if(RecvResult != SOCKET_ERROR)
		{
			BytesRead = RecvResult;
			WriteTo += BytesRead;
			RemainingLength -= BytesRead;
		}
		else
		{
			// TODO: logging
			ReadResult = PlatformSocketReadResult_Error;
			break;
		}
	} while(BytesRead > 0);

	*TotalBytesRead = BufferSize - RemainingLength;
	return ReadResult;
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

void Win32InitOpenGl(HWND Window)
{
	// TODO: review this code and make sure it doesn't leak memory
	HDC WindowDc = GetDC(Window);

	PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
	DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
	DesiredPixelFormat.nVersion = 1;
	DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
	DesiredPixelFormat.dwFlags = (
		PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER
	);
	DesiredPixelFormat.cColorBits = 32;
	DesiredPixelFormat.cAlphaBits = 8;
	DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

	int SuggestedPixelFormatIndex = ChoosePixelFormat(
		WindowDc, &DesiredPixelFormat
	);
	PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
	DescribePixelFormat(
		WindowDc,
		SuggestedPixelFormatIndex,
		sizeof(SuggestedPixelFormat),
		&SuggestedPixelFormat
	);
	SetPixelFormat(WindowDc, SuggestedPixelFormatIndex, &SuggestedPixelFormat);

	GlobalOpenGlrc = wglCreateContext(WindowDc);
	if(wglMakeCurrent(WindowDc, GlobalOpenGlrc))
	{
	}
	else
	{
		ASSERT(false);
		// TODO: Diagnostic
	}
	ReleaseDC(Window, WindowDc);
}

void Win32BufferToWindow(HDC DeviceContext)
{
	SwapBuffers(DeviceContext);
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

win32_window_dimension Win32CalculateWindowDimensions(
	uint32_t WindowWidth, uint32_t WindowHeight
)
{
	RECT ClientRect = {};
	ClientRect.right = WindowWidth;
	ClientRect.bottom = WindowHeight;
	AdjustWindowRect(&ClientRect, WINDOW_STYLE, false);
	win32_window_dimension Result;
	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;
	return Result;
}

void PlatformSetWindowSize(uint32_t WindowWidth, uint32_t WindowHeight)
{
	GlobalWindowWidth = WindowWidth;
	GlobalWindowHeight = WindowHeight;
	win32_window_dimension Dim = Win32CalculateWindowDimensions(
		GlobalWindowWidth, GlobalWindowHeight
	);
	SetWindowPos(
		GlobalWindowHandle,
		HWND_TOP,
		0,
		0,
		Dim.Width,
		Dim.Height,
		SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_FRAMECHANGED
	);
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
			
			Win32BufferToWindow(DeviceContext);

			EndPaint(Window, &Paint);
			break;
		}
		case(WM_GETMINMAXINFO):
		{
			win32_window_dimension Dim = Win32CalculateWindowDimensions(
				GlobalWindowWidth, GlobalWindowHeight
			);
			MINMAXINFO* Mmi = (MINMAXINFO*) LParam;
			Mmi->ptMinTrackSize.x = Dim.Width;
			Mmi->ptMinTrackSize.y = Dim.Height;
			Mmi->ptMaxTrackSize.x = Dim.Width;
			Mmi->ptMaxTrackSize.y = Dim.Height;
			break;
		}
		case(WM_SETCURSOR):
		{
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


// SECTION START: job queue implementation 
struct win32_thread_info
{
	int LogicalThreadIndex;
	platform_job_queue* JobQueue;
};

void PlatformCreateMutex(platform_mutex_handle* Result)
{
	Result->Mutex = CreateMutexA(NULL, false, NULL);
}

void PlatformGetMutex(platform_mutex_handle* MutexHandle)
{
	WaitForSingleObject(MutexHandle->Mutex, INFINITE);
}

void PlatformReleaseMutex(platform_mutex_handle* MutexHandle)
{
	ReleaseMutex(MutexHandle->Mutex);
}

void PlatformFreeMutex(platform_mutex_handle* MutexHandle)
{
	CloseHandle(MutexHandle->Mutex);
	VirtualFree(MutexHandle, 0, MEM_RELEASE);
}

void InitJobQueue(platform_job_queue* JobQueue, uint32_t ThreadCount)
{
	*JobQueue = {};
	heap* JobsToDo = &JobQueue->JobsToDo;
	JobsToDo->Entries = JobQueue->JobsToDoEntries;
	JobsToDo->MaxEntries = JOB_QUEUE_ENTRIES_COUNT;

	JobQueue->EmptyEntriesStart = 0;
	JobQueue->EmptyEntriesCount = JOB_QUEUE_ENTRIES_COUNT;
	for(int Index = 0; Index < JOB_QUEUE_ENTRIES_COUNT; Index++)
	{
		JobQueue->EmptyEntries[Index] = Index;
	}

	JobQueue->UsingJobsToDo = (platform_mutex_handle*) VirtualAlloc(
		0, sizeof(platform_mutex_handle), MEM_COMMIT, PAGE_READWRITE
	);
	JobQueue->UsingJobsToDo->Mutex = CreateMutexA(NULL, false, NULL);
	JobQueue->UsingEmpty = (platform_mutex_handle*) VirtualAlloc(
		0, sizeof(platform_mutex_handle), MEM_COMMIT, PAGE_READWRITE
	);
	JobQueue->UsingEmpty->Mutex = CreateMutexA(NULL, false, NULL);
	JobQueue->FilledSemaphore = (platform_semaphore_handle*) VirtualAlloc(
		0, sizeof(platform_semaphore_handle), MEM_COMMIT, PAGE_READWRITE
	);
	JobQueue->FilledSemaphore->Semaphore = CreateSemaphoreEx(
		0, 0, ARRAY_COUNT(JobQueue->Entries), 0, 0, SEMAPHORE_ALL_ACCESS
	);
	JobQueue->EmptySemaphore = (platform_semaphore_handle*) VirtualAlloc(
		0, sizeof(platform_semaphore_handle), MEM_COMMIT, PAGE_READWRITE
	);
	JobQueue->EmptySemaphore->Semaphore = CreateSemaphoreEx(
		0,
		ARRAY_COUNT(JobQueue->Entries), // NOTE: Initial count
		ARRAY_COUNT(JobQueue->Entries), // NOTE: max count
		0,
		0,
		SEMAPHORE_ALL_ACCESS
	);

	JobQueue->JobDone = (platform_event_handle*) VirtualAlloc(
		0, sizeof(platform_event_handle), MEM_COMMIT, PAGE_READWRITE
	);
	JobQueue->JobDone->Event = CreateEventA(NULL, false, false, NULL);
}

void PlatformAddJob(
	platform_job_queue* JobQueue,
	platform_job_callback* Callback,
	void* Data,
	uint32_t Priority
)
{
	WaitForSingleObject(JobQueue->EmptySemaphore->Semaphore, INFINITE);
	WaitForSingleObject(JobQueue->UsingEmpty->Mutex, INFINITE);
	uint32_t NextEmptyIndex = GetNextEmpty(JobQueue);
	platform_job_queue_entry* Entry = JobQueue->Entries + NextEmptyIndex;
	ReleaseMutex(JobQueue->UsingEmpty->Mutex);

	WaitForSingleObject(JobQueue->UsingJobsToDo->Mutex, INFINITE);
	// NOTE: Entry - JobQueue->Entries gets the index of Entry
	MinInsert(&JobQueue->JobsToDo, Priority, NextEmptyIndex);
	Entry->Callback = Callback;
	Entry->Data = Data;
	ReleaseMutex(JobQueue->UsingJobsToDo->Mutex);
	ReleaseSemaphore(JobQueue->FilledSemaphore->Semaphore, 1, NULL);
}

void PlatformCompleteAllJobs(platform_job_queue* JobQueue)
{
	while(true)
	{
		if(IsEmpty(&JobQueue->JobsToDo))
		{
			break;
		}
		WaitForSingleObject(JobQueue->JobDone->Event, INFINITE);
	}
}

void PlatformCompleteAllJobsAtPriority(
	platform_job_queue* JobQueue, uint32_t Priority
)
{
	heap* JobsToDo = &JobQueue->JobsToDo;
	while(true)
	{
		if(IsEmpty(JobsToDo) || PeekKey(JobsToDo) > Priority)
		{
			break;
		}
		WaitForSingleObject(JobQueue->JobDone->Event, INFINITE);
	}
}

DWORD WINAPI WorkerThread(LPVOID LpParam)
{
	// TODO: We might need a way to join all these threads in case they are
	// CONT: doing something like saving a game
	win32_thread_info* Info = (win32_thread_info*) LpParam;
	platform_job_queue* JobQueue = Info->JobQueue;
	while(true) 
	{
		// NOTE: Get job
		WaitForSingleObject(JobQueue->FilledSemaphore->Semaphore, INFINITE);
		WaitForSingleObject(JobQueue->UsingJobsToDo->Mutex, INFINITE);
		uint32_t EntryIndex = ExtractMinRoot(&JobQueue->JobsToDo);
		platform_job_queue_entry* Entry = JobQueue->Entries + EntryIndex;		
		ReleaseMutex(JobQueue->UsingJobsToDo->Mutex);

		// NOTE: Do job
		Entry->Callback(Entry->Data);
		SetEvent(JobQueue->JobDone->Event);

		// NOTE: open up an entry in the queue
		WaitForSingleObject(JobQueue->UsingEmpty->Mutex, INFINITE);
		AddEmpty(JobQueue, EntryIndex);
		ReleaseMutex(JobQueue->UsingEmpty->Mutex);
		ReleaseSemaphore(JobQueue->EmptySemaphore->Semaphore, 1, NULL);
	}
}
// SECTION STOP: job queue implementation 

int CALLBACK WinMain(
	HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR CommandLine, 
	int ShowCode
)
{
	uint32_t ThreadCount = MAX_THREAD_COUNT - 1;
	HANDLE* ThreadHandles = (HANDLE*) VirtualAlloc(
		0, ThreadCount * sizeof(HANDLE), MEM_COMMIT, PAGE_READWRITE
	);
	win32_thread_info* ThreadInfo = (win32_thread_info*) VirtualAlloc(
		0, ThreadCount * sizeof(win32_thread_info), MEM_COMMIT, PAGE_READWRITE
	);
	platform_job_queue JobQueue;
	InitJobQueue(&JobQueue, ThreadCount);
	for(uint32_t ThreadIndex = 0; ThreadIndex < ThreadCount; ThreadIndex++)
	{
		win32_thread_info* Info = &ThreadInfo[ThreadIndex];
		Info->JobQueue = &JobQueue;
		Info->LogicalThreadIndex = ThreadIndex;
		DWORD ThreadId;
		ThreadHandles[ThreadIndex] = CreateThread( 
			NULL,
			0,
			WorkerThread,
			Info,
			0,
			&ThreadId
		);
	}

	LARGE_INTEGER PerformanceFrequency;
	QueryPerformanceFrequency(&PerformanceFrequency);
	GlobalPerformanceFrequency = PerformanceFrequency.QuadPart;

	// NOTE: set the windows scheduler granularity to 1 ms so that our Sleep()
	// CONT: can be more granular
	UINT DesiredSchedulerMS = 1;
	bool SleepIsGranular = (
		timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR
	);

	// TODO: full screen by default
	GlobalWindowWidth = 800;
	GlobalWindowHeight = 600;

	WNDCLASS WindowClass = {};
	WindowClass.lpfnWndProc = MainWindowCallback;
	WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WindowClass.hInstance = Instance;
	WindowClass.hCursor = LoadCursor(Instance, MAKEINTRESOURCE(32512));
	WindowClass.lpszClassName = "ApocalypseWindowClass";

	// NOTE: if we ever have a variable framerate, we also have to update
	// CONT: sound output config
	win32_debug_time_marker DebugTimeMarkers[30] = {};
	int DebugTimeMarkerIndex = 0;

	if(RegisterClassA(&WindowClass))
	{
		win32_window_dimension WindowDim = Win32CalculateWindowDimensions(
			GlobalWindowWidth, GlobalWindowHeight
		);
		GlobalWindowHandle = CreateWindowExA(
			0,
			WindowClass.lpszClassName,
			"Apocalypse",
			WINDOW_STYLE,
			50,
			50,
			WindowDim.Width,
			WindowDim.Height,
			0,
			0,
			Instance,
			0
		);

		if(GlobalWindowHandle)
		{
			Win32InitOpenGl(GlobalWindowHandle);

			// TODO: query this on Windows
			int MonitorRefreshHz = 60;
			{
				HDC RefreshDC = GetDC(GlobalWindowHandle);
				int Win32RefreshRate = GetDeviceCaps(RefreshDC, VREFRESH);
				ReleaseDC(GlobalWindowHandle, RefreshDC);
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
			GameInitMemory(
				&GameMemory,
				&JobQueue,
				GlobalWindowWidth,
				GlobalWindowHeight
			);

			user_event_index UserEventIndex = 0;
			game_mouse_events MouseEvents = {};
			game_keyboard_events KeyboardEvents = {};

			// NOTE: Since we specified CS_OWNDC, we can just grab this 
			// CONT: once and use it forever. No sharing
			HDC DeviceContext = GetDC(GlobalWindowHandle);
			windows_result_code result = Win32InitDSound(
				GlobalWindowHandle,
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
								GlobalWindowHeight
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
								GlobalWindowHeight
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
								GlobalWindowHeight
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
								GlobalWindowHeight
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
								GlobalWindowHeight
							);
							break;
						}
						case(WM_MOUSEWHEEL):
						{
							Win32WriteMouseEvent(
								&MouseEvents,
								Message.lParam,
								MouseWheel,
								&UserEventIndex,
								GlobalWindowHeight
							);
							game_mouse_event* MouseEvent = (
								&MouseEvents.Events[MouseEvents.Length - 1]
							);
							MouseEvent->WheelScroll = (
								(float) GET_WHEEL_DELTA_WPARAM(Message.wParam) /
								(float) WHEEL_DELTA
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

				GameUpdateAndRender(
					&GameMemory,
					GlobalWindowWidth,
					GlobalWindowHeight,
					&MouseEvents,
					&KeyboardEvents,
					TargetSecondsPerFrame
				);
				// TODO: i think it makes more sense for the reset to occur
				// CONT: at the game layer
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

				HandleGameDebug(
					&GameMemory, GlobalWindowWidth, GlobalWindowHeight
				);

				uint64_t FrameEndCycle = __rdtsc();
				int64_t FrameEndCounter = Win32GetWallClock();

				Win32BufferToWindow(DeviceContext);
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