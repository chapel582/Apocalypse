#ifndef APOCALYPSE_TEXT_INPUT_H

#include "apocalypse_render_group.h"
#include "apocalypse_assets.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_platform.h"
#include "apocalypse_ui.h"

typedef enum
{
	TextInputKbResult_NoAction,
	TextInputKbResult_Submit	
} text_input_kb_result;

typedef enum 
{
	TextInput_ShiftIsDown = 1 << 0,
	TextInput_NewlinesEnabled = 1 << 1,
	TextInput_CharDownDelay = 1 << 2, // NOTE: mutually exclusize from CharDown
	TextInput_CharDown = 1 << 3 // NOTE: mutually exclusize from CharDownDelay
} text_input_flag;

typedef enum 
{
	CursorAlphaState_Static,
	CursorAlphaState_Increasing,
	CursorAlphaState_Decreasing
} cursor_alpha_state;

struct text_input;
typedef void text_input_repeat_callback(text_input* TextInput);

struct text_input
{
	ui_id UiId;

	uint32_t Flags;
	
	uint32_t CursorPos;
	
	uint32_t BufferSize;
	
	float FontHeight;

	char CharDown;
	float RepeatTimer;
	float RepeatDelay;
	float RepeatPeriod;
	
	cursor_alpha_state CursorAlphaState;
	vector4 CursorColor;

	text_input_repeat_callback* RepeatCallback;

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

text_input_kb_result TextInputHandleKeyboard(
	ui_context* UiContext,
	text_input* TextInput,
	game_keyboard_event* KeyboardEvent
);
void TextInputHandleMouse(
	ui_context* UiContext,
	text_input* TextInput,
	rectangle Rectangle,
	game_mouse_event* MouseEvent,
	vector2 MouseEventWorldPos
);
void UpdateTextInput(
	ui_context* UiContext, text_input* TextInput, float DtForFrame
);
void PushTextInput(
	ui_context* UiContext,
	text_input* TextInput,
	vector4 FontColor,
	rectangle Rectangle,
	bitmap_handle Background,
	vector4 BackgroundColor,
	assets* Assets,
	render_group* RenderGroup,
	memory_arena* FrameArena
);

#define APOCALYPSE_TEXT_INPUT_H
#endif