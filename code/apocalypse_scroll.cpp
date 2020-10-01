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

bool CanScroll(scroll_bar* ScrollBar, rectangle* ScrollBox)
{
	return ScrollBar->Rect.Dim.Y < ScrollBox->Dim.Y;
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
	rectangle* ScrollBox,
	game_mouse_event* MouseEvent,
	vector2 MouseEventWorldPos,
	float MinY, 
	float MaxY
)
{
	scroll_handle_mouse_code Result = ScrollHandleMouse_NoAction;
	if(!CanScroll(ScrollBar, ScrollBox))
	{
		return Result;
	}

	rectangle* ScrollBarRect = &ScrollBar->Rect;
	Result = ScrollBoxHandleMouse(
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

void UpdateScrollBarPosDim(
	scroll_bar* ScrollBar,
	rectangle Box,
	float ElementsYStart,
	float AllElementsHeight
)
{
	/* NOTE: 
	for updating the scroll bar's position and dimensions. the scroll bar's 
	position and dimensions are calculated based on where its elements start 
	rendering and the total height of the space containing all its elements
	*/ 
	float BoxTop = GetTop(Box);
	float BoxHeight = Box.Dim.Y;
	rectangle* ScrollBarRect = &ScrollBar->Rect;
	rectangle Trough = ScrollBar->Trough;
	float TroughHeight = Trough.Dim.Y;
	float TroughTop = GetTop(Trough);

	UpdateScrollBarDim(
		ScrollBarRect,
		BoxHeight / AllElementsHeight, 
		TroughHeight
	);

	float TopFractionUnseen = (ElementsYStart - BoxTop) / AllElementsHeight;
	SetTop(ScrollBarRect, TroughTop - TopFractionUnseen * TroughHeight);
}

float GetElementsYStart(
	scroll_bar* ScrollBar, rectangle ScrollBox, float AllElementsHeight
)
{
	/* NOTE: 
	for updating the scroll box elements' positions in world space. we get the
	new y start based on an already updated ScrollBar

	returns 
		float - new y position in world space for starting to render elements
	*/ 
	float BoxTop = GetTop(ScrollBox);
	rectangle ScrollBarRect = ScrollBar->Rect;
	float ScrollBarTop = GetTop(ScrollBarRect);
	rectangle Trough = ScrollBar->Trough;
	float TroughHeight = Trough.Dim.Y;
	float TroughTop = GetTop(Trough);

	float TopFractionUnseen = (TroughTop - ScrollBarTop) / TroughHeight;
	return BoxTop + (TopFractionUnseen * AllElementsHeight);
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