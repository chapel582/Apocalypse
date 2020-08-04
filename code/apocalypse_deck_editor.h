#ifndef APOCALYPSE_DECK_EDITOR_H

#include "apocalypse_card_definitions.h"
#include "apocalypse_rectangle.h"

struct collection_card
{
	rectangle Rectangle;
	bool Active;
	card_definition* Definition;
};

struct deck_editor_state
{
	card_definitions* Definitions;
	collection_card CollectionCards[8];
	uint32_t CurrentFirstCollectionCard;
};

void StartDeckEditor(game_state* GameState);
void UpdateAndRenderDeckEditor(
	game_state* GameState,
	deck_editor_state* SceneState,
	game_offscreen_buffer* BackBuffer,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
);
void StartDeckEditorCallback(void* Data);

#define APOCALYPSE_DECK_EDITOR_H
#endif