#include "apocalypse_deck_storage.h"
#include "apocalypse_platform.h"

#include <stdint.h>

/* NOTE:
	LoadDeck and SaveDeck don't do error handling for missing deck directory. 
	That's handled by installer and potentially repair tool if user messes 
	things up
*/

loaded_deck LoadDeck(char* PathToDeck)
{
	loaded_deck Result;
	
	uint32_t FileSize;
	PlatformGetFileSize(PathToDeck, &FileSize);

	PlatformReadFile(PathToDeck, &Result, FileSize);
	// TODO: handle failure from read file

	return Result;
}

void SaveDeck(char* PathToDeck, loaded_deck* Deck)
{
	PlatformWriteEntireFile(PathToDeck, Deck, sizeof(*Deck));
}