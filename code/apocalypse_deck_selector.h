#ifndef APOCALYPSE_DECK_SELECTOR_H

#include "apocalypse.h"
#include "apocalypse_deck_storage.h"
#include "apocalypse_text_input.h"
#include "apocalypse_button.h"
#include "apocalypse_alert.h"
#include "apocalypse_scroll.h"

struct start_deck_selector_args
{
	scene_type ToStart;
};

struct load_deck_button
{
	ui_id UiId;
};

struct deck_selector_state
{
	scene_type ToStart;

	text_input DeckNameInput;
	rectangle DeckNameInputRectangle;
	char* DeckName;
	uint32_t DeckNameBufferSize;

	load_deck_button LoadDeckButtons[MAX_DECKS_SAVED];
	char* DeckNames;
	uint32_t DeckNamesSize;
	vector2 LoadDeckButtonDim;
	float LoadDeckButtonsYStart;
	float LoadDeckButtonsYMargin;

	scroll_bar DeckScrollBar;
	rectangle DeckScrollBox;

	alert Alert;

	ui_context UiContext;
};

void StartDeckSelectorPrep(game_state* GameState, scene_type ToStart);
void UpdateAndRenderDeckSelector(
	game_state* GameState,
	deck_selector_state* SceneState,
	game_offscreen_buffer* BackBuffer,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
);

#define APOCALYPSE_DECK_SELECTOR_H
#endif