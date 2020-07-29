#ifndef APOCALYPSE_LOAD_DECK_H

#define MAX_CARDS_IN_DECK 60

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

#define APOCALYPSE_LOAD_DECK_H
#endif