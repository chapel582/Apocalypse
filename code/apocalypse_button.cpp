#include "apocalypse_button.h"

#include "apocalypse_platform.h"
#include "apocalypse_rectangle.h"
#include "apocalypse_vector.h"

#include <string.h>

button_handle_event_result ButtonHandleEvent(
	ui_context* UiContext,
	ui_id Id,
	rectangle Rectangle,
	game_mouse_event* MouseEvent,
	vector2 MouseEventWorldPos
)
{
	bool Inside = PointInRectangle(MouseEventWorldPos, Rectangle);
	button_handle_event_result Result = ButtonHandleEvent_NoAction;
	if(IsActive(UiContext, Id))
	{
		if(MouseEvent->Type == PrimaryUp)
		{
			if(IsHot(UiContext, Id) && Inside)
			{
				Result = ButtonHandleEvent_TakeAction;
			}
			else
			{
				ClearActive(UiContext);
			}
		}
	}
	else if(IsHot(UiContext, Id))
	{
		if(MouseEvent->Type == PrimaryDown)
		{
			if(Inside)
			{
				SetActive(UiContext, Id);
			}
		}
	}

	if(Inside)
	{
		SetHot(UiContext, Id);
	}
	else if(IsHot(UiContext, Id))
	{
		ClearHot(UiContext);
	}

	return Result;
}

inline button_handle_event_result ButtonHandleEvent(
	ui_context* UiContext,
	ui_button* Button,
	game_mouse_event* MouseEvent,
	vector2 MouseEventWorldPos
)
{
	return ButtonHandleEvent(
		UiContext,
		Button->Id,
		Button->Rectangle,
		MouseEvent,
		MouseEventWorldPos
	);
}

void PushButtonToRenderGroup(
	rectangle Rectangle,
	bitmap_handle Background,
	render_group* Group,
	assets* Assets, 
	char* Text,
	uint32_t TextBufferSize,
	font_handle Font,
	vector4 TextColor,
	memory_arena* FrameArena
)
{
	vector2 ButtonCenter = GetCenter(Rectangle);
	PushSizedBitmap(
		Group,
		Assets,
		Background,
		ButtonCenter,
		Vector2(Rectangle.Dim.X, 0.0f),
		Vector2(0.0f, Rectangle.Dim.Y),
		Vector4(1.0f, 1.0f, 1.0f, 1.0f)
	);
	if(Text)
	{
		if(*Text != 0)
		{
			ASSERT(FrameArena != NULL);
			PushTextCentered(
				Group,
				Assets,
				Font,
				Text,
				TextBufferSize,
				0.9f * Rectangle.Dim.Y, 
				ButtonCenter,
				TextColor,
				FrameArena 
			);
		}
	}
}