#ifndef APOCALYPSE_BUTTON_H

#include "apocalypse_assets.h"
#include "apocalypse_vector.h"

typedef void button_callback(void* Data);

struct ui_button
{
	bool Active;
	bitmap_handle Background;
	rectangle Rectangle;
	font_handle Font;
	char Text[256];
	vector4 TextColor;
	button_callback* Callback;
	void* Data;
};

void InitButtons(ui_button* Buttons, uint32_t ButtonArrayCount);
void ButtonsHandleMouseEvent(
	ui_button* Buttons,
	uint32_t ButtonArrayCount,
	game_mouse_event* MouseEvent,
	vector2 MouseEventWorldPos
);
void AddButton(
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