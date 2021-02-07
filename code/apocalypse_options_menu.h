#ifndef APOCALYPSE_OPTIONS_MENU_H

#include "apocalypse.h"
#include "apocalypse_ui.h"
#include "apocalypse_dropdown.h"

#define RESOLUTION_CONFIG_PATH "./config/resolution.data"
struct options_menu_state
{
	ui_context UiContext;
	ui_dropdown ResolutionDropdown;
	uint32_t* ResolutionPairs;
};

void StartOptionsMenuPrep(game_state* GameState);
void UpdateAndRenderOptionsMenu(
	game_state* GameState,
	options_menu_state* SceneState,
	uint32_t WindowWidth,
	uint32_t WindowHeight,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
);

#define APOCALYPSE_OPTIONS_MENU_H
#endif APOCALYPSE_OPTIONS_MENU_H
