#ifndef APOCALYPSE_CARD_GAME_H

struct card_game_state
{
	player_id CurrentTurn;
	float TurnTimer;
	player_resources* PlayerResources;
	card* Cards;
	card* SelectedCard;
	int MaxCards;
	deck* Decks;
	card_set* Hands;
	card_set* Tableaus;
	vector2 InfoCardCenter;
	vector2 InfoCardXBound;
	vector2 InfoCardYBound;
	float DisplayMessageUntil;
	char MessageBuffer[256];
	card_definition* Definitions;
	float CardWidth;
	float CardHeight;	
};

#define APOCALYPSE_CARD_GAME_H
#endif