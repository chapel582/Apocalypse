#ifndef APOCALYPSE_SCROLL_H

#include "apocalypse_ui.h"
#include "apocalypse_rectangle.h"
#include "apocalypse_render_group.h"
#include "apocalypse_platform.h"
#include "apocalypse_vector.h"
#include "apocalypse_assets.h"

typedef enum
{
	ScrollBarHandleMouse_NoAction,
	ScrollBarHandleMouse_Moved
} scroll_bar_handle_mouse_code;

struct scroll_bar
{
	ui_id UiId;
	float LastY;
};

inline void InitScrollBar(ui_context* UiContext, scroll_bar* ScrollBar)
{
	ScrollBar->UiId = GetId(UiContext);
	ScrollBar->LastY = 0.0f;
}

scroll_bar_handle_mouse_code ScrollBarHandleMouse(
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

#define APOCALYPSE_SCROLL_H
#endif