#include <windows.h>
#include <stdint.h>

// TODO: this is a global for now
bool GlobalRunning = false;

BITMAPINFO GlobalBitmapInfo = {};
void* GlobalBitmapMemory = NULL;
int GlobalBitmapWidth = 0;
int GlobalBitmapHeight = 0;

void Win32UpdateWindow(HDC DeviceContext, int WindowWidth, int WindowHeight)
{
	StretchDIBits(
		DeviceContext,
		0,
		0,
		GlobalBitmapWidth,
		GlobalBitmapHeight,
		0,
		0,
		WindowWidth,
		WindowHeight,
		GlobalBitmapMemory,
		&GlobalBitmapInfo,
		DIB_RGB_COLORS,
		SRCCOPY
	);
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
			RECT ClientRect = {};
			GetClientRect(Window, &ClientRect);
			GlobalBitmapWidth = ClientRect.right - ClientRect.left;
			GlobalBitmapHeight = ClientRect.bottom - ClientRect.top;

			GlobalBitmapInfo.bmiHeader.biSize = (
				sizeof(GlobalBitmapInfo.bmiHeader)
			);
			GlobalBitmapInfo.bmiHeader.biWidth = GlobalBitmapWidth;
			GlobalBitmapInfo.bmiHeader.biHeight = -GlobalBitmapHeight;
			GlobalBitmapInfo.bmiHeader.biPlanes = 1;
			GlobalBitmapInfo.bmiHeader.biBitCount = 32;
			GlobalBitmapInfo.bmiHeader.biCompression = BI_RGB;

			int BytesPerPixel = 4;
			size_t BitmapMemorySize = (
				(GlobalBitmapWidth * GlobalBitmapHeight) * BytesPerPixel
			);
			if(GlobalBitmapMemory)
			{
				VirtualFree(GlobalBitmapMemory, 0, MEM_RELEASE);
			}
			GlobalBitmapMemory = VirtualAlloc(
				0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE
			);
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
		case(WM_PAINT):
		{
			PAINTSTRUCT Paint = {};
			HDC DeviceContext = BeginPaint(Window, &Paint);
			int WindowWidth = Paint.rcPaint.right - Paint.rcPaint.left;
			int WindowHeight = Paint.rcPaint.bottom - Paint.rcPaint.top;
			
			Win32UpdateWindow(DeviceContext, WindowWidth, WindowHeight);

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
	WNDCLASS WindowClass = {};
	WindowClass.lpfnWndProc = MainWindowCallback;
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
				int BytesPerPixel = 4;
				int Pitch = GlobalBitmapWidth * BytesPerPixel;
				uint8_t* Row = (uint8_t*) GlobalBitmapMemory;
				for(int Y = 0; Y < GlobalBitmapHeight; Y++)
				{
					uint32_t* Pixel = (uint32_t*) Row;
					for(int X = 0; X < GlobalBitmapWidth; X++)
					{
						// *Pixel = 0x000000;
						uint8_t* ColorChannel = (uint8_t*) Pixel;
						*ColorChannel++ = (uint8_t) (X + XOffset);
						*ColorChannel++ = (uint8_t) (Y + YOffset);
						*ColorChannel++ = 0;
						Pixel++;
					}
					Row += Pitch;
				}

				HDC DeviceContext = GetDC(WindowHandle);
				RECT ClientRect = {};
				GetClientRect(WindowHandle, &ClientRect);
				int WindowWidth = ClientRect.right - ClientRect.left;
				int WindowHeight = ClientRect.bottom - ClientRect.top;
				Win32UpdateWindow(DeviceContext, WindowWidth, WindowHeight);
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