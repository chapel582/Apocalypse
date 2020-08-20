#include "apocalypse_scroll.h"

scroll_handle_mouse_code ScrollBoxHandleMouse(
	rectangle* ScrollBarRect,
	rectangle* ScrollBox,
	game_mouse_event* MouseEvent,
	vector2 MouseEventWorldPos,
	float MinY, 
	float MaxY
)
{
	scroll_handle_mouse_code Result = ScrollHandleMouse_NoAction;
	bool Inside = PointInRectangle(MouseEventWorldPos, *ScrollBox);
	if(Inside)
	{
		if(MouseEvent->Type == MouseWheel)
		{
			ScrollBarRect->Min.Y += MouseEvent->WheelScroll * (MaxY - MinY);
			ClampRectY(ScrollBarRect, MinY, MaxY);
			Result = ScrollHandleMouse_Moved;
		}
	}

	return Result;
}

scroll_handle_mouse_code ScrollTroughHandleMouse(
	ui_context* UiContext,
	scroll_bar* ScrollBar,
	rectangle* ScrollBarRect,
	rectangle* ScrollTrough,
	game_mouse_event* MouseEvent,
	vector2 MouseEventWorldPos,
	float MinY, 
	float MaxY
)
{
	scroll_handle_mouse_code Result = ScrollHandleMouse_NoAction;
	bool Inside = PointInRectangle(MouseEventWorldPos, *ScrollTrough);
	ui_id BarId = ScrollBar->UiId;

	if(Inside && !IsHot(UiContext, BarId))
	{
		if(MouseEvent->Type == PrimaryDown)
		{
			if(MouseEventWorldPos.Y < GetBottom(*ScrollBarRect))
			{
				SetTop(ScrollBarRect, GetBottom(*ScrollBarRect));
			}
			else if(MouseEventWorldPos.Y > GetTop(*ScrollBarRect))
			{
				SetBottom(ScrollBarRect, GetTop(*ScrollBarRect));
			}
			ClampRectY(ScrollBarRect, MinY, MaxY);
			Result = ScrollHandleMouse_Moved;
		}
	}

	return Result;
}

scroll_handle_mouse_code ScrollBarHandleMouse(
	ui_context* UiContext,
	scroll_bar* ScrollBar,
	rectangle* Rectangle,
	game_mouse_event* MouseEvent,
	vector2 MouseEventWorldPos,
	float MinY, 
	float MaxY
)
{
	scroll_handle_mouse_code Result = ScrollHandleMouse_NoAction;
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
			ClampRectY(Rectangle, MinY, MaxY);
			Result = ScrollHandleMouse_Moved;
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

scroll_handle_mouse_code ScrollHandleMouse(
	ui_context* UiContext,
	scroll_bar* ScrollBar,
	rectangle* ScrollBarRect,
	rectangle* ScrollBox,
	game_mouse_event* MouseEvent,
	vector2 MouseEventWorldPos,
	float MinY, 
	float MaxY
)
{
	scroll_handle_mouse_code Result = ScrollBoxHandleMouse(
		ScrollBarRect, ScrollBox, MouseEvent, MouseEventWorldPos, MinY, MaxY
	);
	if(Result == ScrollHandleMouse_Moved)
	{
		return Result;
	}

	Result = ScrollBarHandleMouse(
		UiContext,
		ScrollBar,
		ScrollBarRect,
		MouseEvent,
		MouseEventWorldPos,
		MinY,
		MaxY
	);
	if(Result == ScrollHandleMouse_Moved)
	{
		return Result;
	}

	rectangle ScrollTrough = MakeRectangle(
		Vector2(ScrollBarRect->Min.X, MinY),
		Vector2(ScrollBarRect->Dim.X, MaxY - MinY)
	);
	Result = ScrollTroughHandleMouse(
		UiContext,
		ScrollBar,
		ScrollBarRect,
		&ScrollTrough,
		MouseEvent,
		MouseEventWorldPos,
		MinY, 
		MaxY
	);
	if(Result == ScrollHandleMouse_Moved)
	{
		return Result;
	}

	return ScrollHandleMouse_NoAction;
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