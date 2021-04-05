#ifndef APOCALYPSE_BUTTON_H

#include "apocalypse_assets.h"
#include "apocalypse_vector.h"
#include "apocalypse_platform.h"
#include "apocalypse_ui.h"
#include "apocalypse_rectangle.h"

struct ui_button
{
	ui_id Id;
	rectangle* Rectangle;
};

inline void InitButton(
	ui_context* Context, ui_button* Button, rectangle* Rectangle
)
{
	Button->Id = GetId(Context);
	Button->Rectangle = Rectangle;
}

typedef enum
{
	ButtonHandleEvent_TakeAction,
	ButtonHandleEvent_NoAction
} button_handle_event_result;

button_handle_event_result ButtonHandleEvent(
	ui_context* UiContext,
	ui_button* Button,
	mouse_event_type MouseEventType,
	vector2 MouseEventWorldPos
);
button_handle_event_result ButtonHandleEvent(
	ui_context* UiContext,
	ui_id Id,
	rectangle Rectangle,
	mouse_event_type MouseEventType,
	vector2 MouseEventWorldPos
);
void PushButtonToRenderGroup(
	rectangle Rectangle,
	bitmap_handle Background,
	render_group* Group,
	assets* Assets, 
	char* Text,
	uint32_t TextBufferSize,
	font_handle Font,
	float FontSize,
	vector4 TextColor,
	memory_arena* FrameArena,
	uint32_t Layer = 1
);
void PushButtonToRenderGroup(
	rectangle Rectangle,
	bitmap_handle Background,
	render_group* Group,
	assets* Assets, 
	char* Text,
	uint32_t TextBufferSize,
	font_handle Font,
	vector4 TextColor,
	memory_arena* FrameArena,
	uint32_t Layer = 1
);

#define APOCALYPSE_BUTTON_H

#endif