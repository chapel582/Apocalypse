#ifndef APOCALYPSE_MAIN_MENU_H

#include "apocalypse_button.h"
#include "apocalypse_rectangle.h"

struct main_menu_state
{
	ui_context UiContext;
	ui_button CardGameButton;
	ui_button DeckEditorButton;
};

void StartMainMenu(
	game_state* GameState, uint32_t WindowWidth, uint32_t WindowHeight
);

#define APOCALYPSE_MAIN_MENU_H
#endif