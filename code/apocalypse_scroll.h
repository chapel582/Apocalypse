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

	rectangle ScrollBox;
	uint32_t ScrollBoxClipIndex;
};

inline void InitScrollBar(
	ui_context* UiContext,
	scroll_bar* ScrollBar,
	float ScrollBarWidth,
	rectangle ScrollBox,
	uint32_t ScrollBoxClipIndex = 0
)
{
	*ScrollBar = {};
	ScrollBar->UiId = GetId(UiContext);
	ScrollBar->LastY = 0.0f;
	ScrollBar->ScrollBox = ScrollBox;
	
	// NOTE: the + 1.0f is here so the scroll bar doesn't render initially
	vector2 StackScrollBarDim = Vector2(ScrollBarWidth, ScrollBox.Dim.Y + 1.0f);
	ScrollBar->Rect = MakeRectangle(Vector2(0, 0), StackScrollBarDim);
	SetTopLeft(&ScrollBar->Rect, GetTopRight(ScrollBox));

	ScrollBar->Trough = MakeRectangle(
		Vector2(0, 0),
		Vector2(ScrollBar->Rect.Dim.X, ScrollBox.Dim.Y)
	);
	SetTopLeft(
		&ScrollBar->Trough, GetTopRight(ScrollBox)
	);
	

	ScrollBar->ScrollBoxClipIndex = ScrollBoxClipIndex;
}

bool CanScroll(scroll_bar* ScrollBar);
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
	game_mouse_event* MouseEvent,
	vector2 MouseEventWorldPos,
	float MinY, 
	float MaxY
);
void UpdateScrollBarPosDim(
	scroll_bar* ScrollBar,
	float ElementsYStart,
	float AllElementsHeight
);
float GetElementsYStart(scroll_bar* ScrollBar, float AllElementsHeight);
#define APOCALYPSE_SCROLL_H
#endif