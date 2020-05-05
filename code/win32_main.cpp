#include <windows.h>
#include <stdint.h>

// TODO: this is a global for now
bool GlobalRunning = false;

struct win32_offscreen_buffer
{
	int Width;
	int Height;
	int Pitch;
	void* Memory;
	BITMAPINFO Info;
};

win32_offscreen_buffer GlobalBackBuffer;

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
	GlobalBackBuffer = {};

	WNDCLASS WindowClass = {};
	WindowClass.lpfnWndProc = MainWindowCallback;
	WindowClass.style = CS_HREDRAW | CS_VREDRAW;
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
			int XOffset = 0;
			int YOffset = 0;

			GlobalRunning = true;
			while(GlobalRunning)
			{
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


				// NOTE: this is currently our render loop
				// NOTE: it will be removed soon
				uint8_t* Row = (uint8_t*) GlobalBackBuffer.Memory;
				for(int Y = 0; Y < GlobalBackBuffer.Height; Y++)
				{
					uint32_t* Pixel = (uint32_t*) Row;
					for(int X = 0; X < GlobalBackBuffer.Width; X++)
					{
						// *Pixel = 0x000000;
						uint8_t* ColorChannel = (uint8_t*) Pixel;
						*ColorChannel++ = (uint8_t) (X + XOffset);
						*ColorChannel++ = (uint8_t) (Y + YOffset);
						*ColorChannel++ = 0;
						Pixel++;
					}
					Row += GlobalBackBuffer.Pitch;
				}

				HDC DeviceContext = GetDC(WindowHandle);
				win32_window_dimension Dimensions = GetWindowDimension(
					WindowHandle
				);
				Win32BufferToWindow(
					DeviceContext, Dimensions.Width, Dimensions.Height
				);
				XOffset++;
				YOffset++;
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