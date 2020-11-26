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

	bool NetworkGame;
	bool IsLeader;
	platform_socket ListenSocket;
	platform_socket ConnectSocket;
};

struct load_deck_button
{
	ui_id UiId;
};

struct deck_selector_state
{
	scene_type ToStart;

	vector2 ScreenDimInWorld;

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

	alert Alert;

	ui_context UiContext;

	// NOTE: these options exist for network games
	loaded_deck P1Deck;
	loaded_deck P2Deck;

	bool WaitingForOpponent;
	bool IsLeader;
	bool NetworkGame;
	platform_socket ListenSocket;
	platform_socket ConnectSocket;
};

void StartDeckSelectorPrep(
	game_state* GameState,
	scene_type ToStart,
	bool NetworkGame,
	bool IsLeader,
	platform_socket* ListenSocket,
	platform_socket* ConnectSocket
);
void UpdateAndRenderDeckSelector(
	game_state* GameState,
	deck_selector_state* SceneState,
	uint32_t WindowWidth,
	uint32_t WindowHeight,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
);

#define APOCALYPSE_DECK_SELECTOR_H
#endif