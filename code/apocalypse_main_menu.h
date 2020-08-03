#ifndef APOCALYPSE_MAIN_MENU_H

#include "apocalypse_button.h"

struct main_menu_state
{
	ui_button Buttons[16];
};

void StartMainMenu(game_state* GameState, game_offscreen_buffer* BackBuffer);

#define APOCALYPSE_MAIN_MENU_H
#endif