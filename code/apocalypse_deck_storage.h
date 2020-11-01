#ifndef APOCALYPSE_LOAD_DECK_H

#define MAX_CARDS_IN_DECK 60
#define MAX_DECKS_SAVED 1024

#pragma pack(push, 1)
struct loaded_deck_header
{
	uint8_t CardCount;
};

struct loaded_deck
{
	loaded_deck_header Header;
	uint32_t Ids[MAX_CARDS_IN_DECK];
};
#pragma pack(pop)

void SaveDeck(char* PathToDeck, loaded_deck* Deck);
void DeleteDeck(char* PathToDeck);
loaded_deck LoadDeck(char* PathToDeck);
void FormatDeckPath(char* Buffer, uint32_t MaxBufferSize, char* DeckName);
void GetAllDeckPaths(char* Buffer, uint32_t MaxBufferSize);

#define APOCALYPSE_LOAD_DECK_H
#endif