#ifndef APOCALYPSE_DECK_EDITOR_H

#include "apocalypse_card_definitions.h"
#include "apocalypse_rectangle.h"
#include "apocalypse_button.h"
#include "apocalypse_deck_storage.h"

#include <string.h> // TODO: for text_input. move when that file is made

typedef enum 
{
	TextInput_Active = 1 << 0,
	TextInput_Selected = 1 << 1,
	TextInput_ShiftIsDown = 1 << 2,
	TextInput_NewlinesEnabled = 1 << 3,
} text_input_flag;

typedef void text_input_callback(void* Data);

struct text_input
{
	uint32_t Flags;
	uint32_t CursorPos;
	uint32_t BufferSize;
	rectangle Rectangle;
	float FontHeight;
	char* Buffer;
	text_input_callback* SubmitCallback;
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

struct standard_submit_args
{
	uint32_t DataLength;
	text_input* TextInput;
	char* Buffer;
	char* Dest;
};

void StandardSubmit(void* Data)
{
	standard_submit_args* Args = (standard_submit_args*) Data;
	memcpy(Args->Dest, Args->Buffer, Args->DataLength);
	ClearFlag(Args->TextInput, TextInput_Active);
}

struct collection_card
{
	uint32_t ButtonHandle;
	card_definition* Definition;
};

inline bool IsActive(collection_card* CollectionCard)
{
	return CollectionCard->Definition != NULL;
}

struct deck_editor_card
{
	uint32_t ButtonHandle;
	card_definition* Definition; 
};

inline bool IsActive(deck_editor_card* DeckCard)
{
	return DeckCard->Definition != NULL;
}

struct deck_editor_cards
{
	deck_editor_card Cards[MAX_CARDS_IN_DECK];
	uint32_t ActiveCardCount;
	float XPos;
	float YStart;
	float YMargin;
	vector2 Dim;
};

#define COLLECTION_CARDS_DISPLAYED 8
struct deck_editor_state
{
	ui_button CollectionButtons[COLLECTION_CARDS_DISPLAYED];
	ui_button DeckButtons[MAX_CARDS_IN_DECK];
	card_definitions* Definitions;
	collection_card CollectionCards[COLLECTION_CARDS_DISPLAYED];
	deck_editor_cards DeckCards;
	uint32_t CurrentFirstCollectionCard;
	text_input DeckNameInput;
	char* DeckName;
	uint32_t DeckNameBufferSize;
	bool DeckNameSet;
	vector2 InfoCardCenter;
	vector2 InfoCardXBound;
	vector2 InfoCardYBound;
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