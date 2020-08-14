#include "apocalypse_button.h"

#include "apocalypse_platform.h"
#include "apocalypse_rectangle.h"
#include "apocalypse_vector.h"

#include <string.h>

void InitButtons(ui_button* Buttons, uint32_t ButtonArrayCount)
{
	for(
		uint32_t ButtonIndex = 0;
		ButtonIndex < ButtonArrayCount;
		ButtonIndex++
	)
	{
		ui_button* Button = &Buttons[ButtonIndex];
		ClearAllFlags(Button);
	}
}

void ButtonsHandleMouseEvent(
	ui_button* Buttons,
	uint32_t ButtonArrayCount,
	game_mouse_event* MouseEvent,
	vector2 MouseEventWorldPos
)
{
	for(
		uint32_t ButtonIndex = 0;
		ButtonIndex < ButtonArrayCount;
		ButtonIndex++
	)
	{
		ui_button* Button = &Buttons[ButtonIndex];
		if(CheckFlag(Button, UiButton_Interactable))
		{
			if(PointInRectangle(MouseEventWorldPos, Button->Rectangle))
			{
				if(MouseEvent->Type == PrimaryUp)
				{
					Button->Callback(Button->Data);
					break;
				}
				else if(MouseEvent->Type == MouseMove)
				{
					SetFlag(Button, UiButton_HoveredOver);
					break;
				}
			}
			else
			{
				ClearFlag(Button, UiButton_HoveredOver);
			}
		}		
	}
}

ui_button* AddButton(
	ui_button* Buttons,
	uint32_t ButtonArrayCount,
	rectangle Rectangle,
	bitmap_handle Background,
	font_handle Font,
	char* Text,
	vector4 TextColor,
	button_callback* Callback,
	void* Data
)
{
	ui_button* ButtonToUse = NULL;
	uint32_t ButtonIndex;
	for(
		ButtonIndex = 0;
		ButtonIndex < ButtonArrayCount;
		ButtonIndex++
	)
	{
		ui_button* Button = &Buttons[ButtonIndex];
		if(!CheckFlag(Button, UiButton_Allocated))
		{
			ButtonToUse = Button;
			break;
		}
	}
	ASSERT(ButtonToUse != NULL);
	SetFlag(ButtonToUse, UiButton_Allocated);
	ButtonToUse->Rectangle = Rectangle;
	ButtonToUse->Callback = Callback;
	ButtonToUse->Data = Data;
	ButtonToUse->Background = Background;
	ButtonToUse->Font = Font;
	ButtonToUse->TextColor = TextColor;
	if(Text != NULL)
	{
		strcpy_s(ButtonToUse->Text, ARRAY_COUNT(ButtonToUse->Text), Text);
	}
	return ButtonToUse;
}

void AddButtonsFlags(
	ui_button* Buttons, uint32_t ButtonArrayCount, uint32_t Flags
)
{
	for(
		uint32_t ButtonIndex = 0;
		ButtonIndex < ButtonArrayCount;
		ButtonIndex++
	)
	{
		ui_button* Button = &Buttons[ButtonIndex];
		if(CheckFlag(Button, UiButton_Allocated))
		{
			Button->Flags |= Flags;
		}
	}
}

void PushButtonToRenderGroup(
	ui_button* Button,
	render_group* Group,
	assets* Assets,
	memory_arena* FrameArena, 
	vector4 Color
)
{
	if(CheckFlag(Button, UiButton_Visible))
	{
		vector2 ButtonCenter = GetCenter(Button->Rectangle);
		PushSizedBitmap(
			Group,
			Assets,
			Button->Background,
			ButtonCenter,
			Vector2(Button->Rectangle.Dim.X, 0.0f),
			Vector2(0.0f, Button->Rectangle.Dim.Y),
			Color
		);
		if(*Button->Text != 0)
		{
			PushTextCentered(
				Group,
				Assets,
				Button->Font,
				Button->Text,
				ARRAY_COUNT(Button->Text),
				0.9f * Button->Rectangle.Dim.Y, 
				ButtonCenter,
				Button->TextColor,
				FrameArena 
			);
		}
	}
}

void PushButtonsToRenderGroup(
	ui_button* Buttons,
	uint32_t ButtonArrayCount,
	render_group* Group,
	assets* Assets, 
	memory_arena* FrameArena
)
{
	vector4 White = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	for(
		uint32_t ButtonIndex = 0;
		ButtonIndex < ButtonArrayCount;
		ButtonIndex++
	)
	{
		ui_button* Button = &Buttons[ButtonIndex];
		PushButtonToRenderGroup(Button, Group, Assets, FrameArena, White);
	}
}