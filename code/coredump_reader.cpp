#include "apocalypse_platform.h"
#include "apocalypse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

int main(int Argc, char* Argv[], char* Envp[])
{
	if(Argc != 2)
	{
		printf("Cannot open coredump file. Wrong number of arguments\n");
		printf("coredump_reader.exe <permanentcoredumppath>\n");
		return 1;
	}
	FILE* CoredumpFd = NULL;
	fopen_s(&CoredumpFd, Argv[1], "rb");

	fseek(CoredumpFd, 0, SEEK_END);
	size_t CoredumpSize = ftell(CoredumpFd);
	fseek(CoredumpFd, 0, SEEK_SET);
	LPVOID BaseAddress = (LPVOID) TERABYTES(2);
	void* Memory = VirtualAlloc(
		BaseAddress, CoredumpSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE
	);
	size_t BytesRead = fread(Memory, 1, CoredumpSize, CoredumpFd);
	printf("BytesRead: %zd\n", BytesRead);
	game_state* GameState = (game_state*) Memory;

	return 0;
}