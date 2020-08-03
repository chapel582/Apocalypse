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
		Button->Active = false;
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
		if(
			Button->Active && 
			MouseEvent->Type == PrimaryUp && 
			PointInRectangle(MouseEventWorldPos, Button->Rectangle)
		)
		{
			Button->Callback(Button->Data);
			break;
		}		
	}
}

void AddButton(
	ui_button* Buttons,
	uint32_t ButtonArrayCount,
	rectangle Rectangle,
	bitmap_handle Background,
	font_handle Font,
	char* Text,
	button_callback* Callback,
	void* Data
)
{
	ui_button* ButtonToUse = NULL;
	for(
		uint32_t ButtonIndex = 0;
		ButtonIndex < ButtonArrayCount;
		ButtonIndex++
	)
	{
		ui_button* Button = &Buttons[ButtonIndex];
		if(!Button->Active)
		{
			Button->Active = true;
			ButtonToUse = Button;
			break;
		}	
	}
	ASSERT(ButtonToUse != NULL);
	strcpy_s(ButtonToUse->Text, ARRAY_COUNT(ButtonToUse->Text), Text);
	ButtonToUse->Rectangle = Rectangle;
	ButtonToUse->Callback = Callback;
	ButtonToUse->Data = Data;
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
		if(Button->Active)
		{
			vector2 ButtonCenter = GetCenter(Button->Rectangle);
			PushCenteredBitmap(
				Group,
				Assets,
				Button->Background,
				ButtonCenter,
				Vector2(Button->Rectangle.Dim.X, 0.0f),
				Vector2(0.0f, Button->Rectangle.Dim.Y),
				White
			);
			PushTextCentered(
				Group,
				Assets,
				Button->Font,
				Button->Text,
				ARRAY_COUNT(Button->Text),
				0.9f * Button->Rectangle.Dim.Y, 
				ButtonCenter,
				White,
				FrameArena 
			);
		}	
	}
}