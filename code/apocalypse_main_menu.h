#ifndef APOCALYPSE_MAIN_MENU_H

#include "apocalypse_button.h"
#include "apocalypse_rectangle.h"

struct main_menu_state
{
	ui_context UiContext;
	ui_button CardGameButton;
	ui_button DeckEditorButton;
};

void StartMainMenu(game_state* GameState, game_offscreen_buffer* BackBuffer);

#define APOCALYPSE_MAIN_MENU_H
#endif