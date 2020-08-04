#ifndef APOCALYPSE_DECK_EDITOR_H

struct deck_editor_state
{
	card_definitions* Definitions;
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