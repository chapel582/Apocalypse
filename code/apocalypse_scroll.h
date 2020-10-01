#ifndef APOCALYPSE_SCROLL_H

#include "apocalypse_ui.h"
#include "apocalypse_rectangle.h"
#include "apocalypse_render_group.h"
#include "apocalypse_platform.h"
#include "apocalypse_vector.h"
#include "apocalypse_assets.h"

typedef enum
{
	ScrollHandleMouse_NoAction,
	ScrollHandleMouse_Moved
} scroll_handle_mouse_code;

struct scroll_bar
{
	ui_id UiId;
	rectangle Rect;
	rectangle Trough;
	float LastY;
};

inline void InitScrollBar(ui_context* UiContext, scroll_bar* ScrollBar)
{
	*ScrollBar = {};
	ScrollBar->UiId = GetId(UiContext);
	ScrollBar->LastY = 0.0f;
}

bool CanScroll(scroll_bar* ScrollBar, rectangle* ScrollBox);
scroll_handle_mouse_code ScrollBoxHandleMouse(
	rectangle* ScrollBarRect,
	rectangle* ScrollBox,
	game_mouse_event* MouseEvent,
	vector2 MouseEventWorldPos,
	float MinY, 
	float MaxY
);
scroll_handle_mouse_code ScrollBarHandleMouse(
	ui_context* UiContext,
	scroll_bar* ScrollBar,
	rectangle* Rectangle,
	game_mouse_event* MouseEvent,
	vector2 MouseEventWorldPos,
	float MinY, // NOTE: rectangle y min can't go below this
	float MaxY // NOTE: rectangle y max can't go above this
);
void UpdateScrollBarDim(
	rectangle* Rectangle, float FractionShown, float MaxDimY
);
void PushScrollBarToRenderGroup(
	rectangle Rectangle,
	bitmap_handle ScrollBarBitmap,
	render_group* Group,
	assets* Assets
);
scroll_handle_mouse_code ScrollHandleMouse(
	ui_context* UiContext,
	scroll_bar* ScrollBar,
	rectangle* ScrollBox,
	game_mouse_event* MouseEvent,
	vector2 MouseEventWorldPos,
	float MinY, 
	float MaxY
);
float GetElementsYStart(
	scroll_bar* ScrollBar, rectangle ScrollBox, float AllElementsHeight
);
#define APOCALYPSE_SCROLL_H
#endif