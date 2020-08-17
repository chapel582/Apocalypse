#include "apocalypse_scroll.h"

scroll_bar_handle_mouse_code ScrollBarHandleMouse(
	ui_context* UiContext,
	scroll_bar* ScrollBar,
	rectangle* Rectangle,
	game_mouse_event* MouseEvent,
	vector2 MouseEventWorldPos,
	float MinY, 
	float MaxY
)
{
	scroll_bar_handle_mouse_code Result = ScrollBarHandleMouse_NoAction;
	bool Inside = PointInRectangle(MouseEventWorldPos, *Rectangle);
	ui_id Id = ScrollBar->UiId;

	if(IsActive(UiContext, Id))
	{
		if(MouseEvent->Type == PrimaryUp)
		{
			ClearActive(UiContext);
		}
		else
		{
			Rectangle->Min.Y += MouseEventWorldPos.Y - ScrollBar->LastY;
			ScrollBar->LastY = MouseEventWorldPos.Y;
			if(GetBottom(*Rectangle) < MinY)
			{
				SetBottom(Rectangle, MinY);
			}
			if(GetTop(*Rectangle) > MaxY)
			{
				SetTop(Rectangle, MaxY);				
			}
			Result = ScrollBarHandleMouse_Moved;
		}
	}
	else if(IsHot(UiContext, Id))
	{
		if(MouseEvent->Type == PrimaryDown)
		{
			if(Inside)
			{
				SetActive(UiContext, Id);
				ScrollBar->LastY = MouseEventWorldPos.Y;
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

void UpdateScrollBarDim(
	rectangle* Rectangle, float FractionShown, float MaxDimY
)
{
	if(FractionShown > 1.0f)
	{
		FractionShown = 1.0f;
	}
	else if(FractionShown < 0.0f)
	{
		FractionShown = 0.0f;
	}
	Rectangle->Dim.Y = FractionShown * MaxDimY;
}

void PushScrollBarToRenderGroup(
	rectangle Rectangle,
	bitmap_handle ScrollBarBitmap,
	render_group* Group,
	assets* Assets
)
{
	if(Rectangle.Dim.Y > 0.0f)
	{
		vector2 ScrollBarCenter = GetCenter(Rectangle);
		PushSizedBitmap(
			Group,
			Assets,
			ScrollBarBitmap,
			ScrollBarCenter,
			Vector2(Rectangle.Dim.X, 0.0f),
			Vector2(0.0f, Rectangle.Dim.Y),
			Vector4(1.0f, 1.0f, 1.0f, 1.0f)
		);
	}
}