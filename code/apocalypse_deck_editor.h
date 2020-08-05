#ifndef APOCALYPSE_DECK_EDITOR_H

#include "apocalypse_card_definitions.h"
#include "apocalypse_rectangle.h"

typedef enum 
{
	TextInput_Active = 1 << 0,
	TextInput_Selected = 1 << 1,
	TextInput_ShiftIsDown = 1 << 2,
	TextInput_NewlinesEnabled = 1 << 3,
} text_input_flag;

struct text_input
{
	uint32_t Flags;
	uint32_t CursorPos;
	uint32_t BufferSize;
	rectangle Rectangle;
	float FontHeight;
	char* Buffer;
};

inline bool CheckFlag(text_input* TextInput, text_input_flag Flag)
{
	return (TextInput->Flags & Flag) > 0;
}

inline void SetFlag(text_input* TextInput, text_input_flag Flag)
{
	TextInput->Flags |= Flag;
}

inline void ClearFlag(text_input* TextInput, text_input_flag Flag)
{
	TextInput->Flags &= ~Flag;
}

inline void ClearAllFlags(text_input* TextInput)
{
	TextInput->Flags = 0;
}

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
	text_input DeckNameInput;
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
void StartDeckEditorCallback(void* Data);

#define APOCALYPSE_DECK_EDITOR_H
#endif