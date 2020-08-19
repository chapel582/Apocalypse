#ifndef APOCALYPSE_DECK_EDITOR_H

#include "apocalypse_card_definitions.h"
#include "apocalypse_rectangle.h"
#include "apocalypse_button.h"
#include "apocalypse_text_input.h"
#include "apocalypse_deck_storage.h"
#include "apocalypse_alert.h"
#include "apocalypse_scroll.h"

struct collection_card
{
	ui_button Button;
};

struct deck_editor_card
{
	uint32_t Count;
	ui_id UiId;
	card_definition* Definition; 
};

inline bool IsActive(deck_editor_card* DeckCard)
{
	return DeckCard->Definition != NULL;
}

struct deck_editor_cards
{
	deck_editor_card Cards[MAX_CARDS_IN_DECK];
	float XPos;
	float YStart;
	float YMargin;
	vector2 Dim;
	uint32_t ActiveButtons;
};

struct load_deck_button
{
	ui_id UiId;
};

#define COLLECTION_CARDS_DISPLAYED 8
struct deck_editor_state
{
	card_definitions* Definitions;

	// NOTE: collection card stuff
	collection_card CollectionCards[COLLECTION_CARDS_DISPLAYED];
	int32_t CollectionStartIndex;
	uint32_t NumRows;
	uint32_t NumCols;
	vector2 CollectionCardDim;
	float XOffset;
	float YOffset;
	float XMargin;
	float YMargin;

	deck_editor_cards DeckCards;
	
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

	vector2 DeckNamePos;
	bool DeckNameSet;

	vector2 InfoCardCenter;
	vector2 InfoCardXBound;
	vector2 InfoCardYBound;

	alert Alert;

	ui_context UiContext;
	ui_button SaveButton;
	ui_button CollectionPrev;
	ui_button CollectionNext;

	scroll_bar DeckScrollBar;
	rectangle DeckScrollBarRect;
	float DeckScrollBarTop;
	float MaxDeckScrollBarY;
};

void StartDeckEditor(game_state* GameState, game_offscreen_buffer* BackBuffer);
void UpdateAndRenderDeckEditor(
	game_state* GameState,
	deck_editor_state* SceneState,
	game_offscreen_buffer* BackBuffer,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
);

#define APOCALYPSE_DECK_EDITOR_H
#endif