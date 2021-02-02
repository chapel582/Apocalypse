#ifndef APOCALYPSE_DROPDOWN_H

#include "apocalypse_platform.h"
#include "apocalypse_button.h"
#include "apocalypse_scroll.h"

typedef enum 
{
	DropdownHandleEvent_NoAction,
	DropdownHandleEvent_Primary
} dropdown_handle_event_result;

struct ui_dropdown
{
	ui_button DropButton;
	ui_button* DropdownButtons;
	uint32_t DropdownButtonsLen;
	uint32_t ButtonsMaxLen;
	bool DropdownVisible;
};

inline void InitDropdown(
	ui_context* Context,
	ui_dropdown* Dropdown,
	rectangle* Rectangle,
	memory_arena* Arena,
	uint32_t ButtonsMaxLen
)
{
	*Dropdown = {};
	InitButton(
		Context, &(Dropdown->DropButton), Rectangle
	);
	Dropdown->DropdownButtons = PushArray(Arena, ButtonsMaxLen, ui_button);
	Dropdown->ButtonsMaxLen = ButtonsMaxLen;
}

void AddDropdownEntry(
	game_state* GameState,
	ui_context* Context,
	ui_dropdown* Dropdown
)
{	
	rectangle* DropButtonRect = Dropdown->DropButton.Rectangle;
	float YMin = (
		DropButtonRect->Min.Y -
		(Dropdown->DropdownButtonsLen + 1) * DropButtonRect->Dim.Y
	);
	float XMin = DropButtonRect->Min.X;
	
	rectangle* NextRectangle = MakeTrackedRectangle(
		GameState, Vector2(XMin, YMin), DropButtonRect->Dim
	);
	ui_button* Button = (
		&Dropdown->DropdownButtons[Dropdown->DropdownButtonsLen]
	);
	InitButton(Context, Button, NextRectangle);

	Dropdown->DropdownButtonsLen++;
	ASSERT(Dropdown->DropdownButtonsLen <= Dropdown->ButtonsMaxLen);
}

void PushDropdownToRenderGroup(
	rectangle Rectangle,
	bitmap_handle Background,
	render_group* Group,
	assets* Assets, 
	char* Text,
	uint32_t TextBufferSize,
	font_handle Font,
	vector4 TextColor,
	memory_arena* FrameArena
);

#define APOCALYPSE_DROPDOWN_H

#endif