#include <windows.h>

int CALLBACK WinMain(
	HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR CommandLine, 
	int ShowCode
)
{
	MessageBoxA(
		0, "This is a message box", "Apocalypse", MB_OK | MB_ICONINFORMATION
	);
	return 0;
}