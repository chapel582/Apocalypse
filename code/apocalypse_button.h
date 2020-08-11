#ifndef APOCALYPSE_BUTTON_H

#include "apocalypse_assets.h"
#include "apocalypse_vector.h"

typedef void button_callback(void* Data);
typedef enum 
{
	UiButton_Active = 1 << 0,
	UiButton_HoveredOver = 1 << 1
} ui_button_flag;

struct ui_button
{
	uint32_t Flags;
	bitmap_handle Background;
	rectangle Rectangle;
	font_handle Font;
	char Text[256];
	vector4 TextColor;
	button_callback* Callback;
	void* Data;
};

inline void SetFlag(ui_button* Button, ui_button_flag Flag)
{
	Button->Flags |= Flag; 
}

inline void ClearFlag(ui_button* Button, ui_button_flag Flag)
{
	Button->Flags &= ~Flag;
}

inline void ClearAllFlags(ui_button* Button)
{
	Button->Flags = 0;
}

inline bool CheckFlag(ui_button* Button, ui_button_flag Flag)
{
	return (Button->Flags & Flag) > 0;
}

void InitButtons(ui_button* Buttons, uint32_t ButtonArrayCount);
void ButtonsHandleMouseEvent(
	ui_button* Buttons,
	uint32_t ButtonArrayCount,
	game_mouse_event* MouseEvent,
	vector2 MouseEventWorldPos
);
ui_button* AddButton(
	ui_button* Buttons,
	uint32_t ButtonArrayCount,
	rectangle Rectangle,
	bitmap_handle Background,
	font_handle Font,
	char* Text,
	vector4 TextColor,
	button_callback* Callback,
	void* Data
);
void PushButtonsToRenderGroup(
	ui_button* Buttons,
	uint32_t ButtonArrayCount,
	render_group* Group,
	assets* Assets, 
	memory_arena* FrameArena
);

#define APOCALYPSE_BUTTON_H

#endif