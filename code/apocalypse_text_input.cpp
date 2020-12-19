#include "apocalypse_text_input.h"

void InitTextInput(
	ui_context* UiContext,
	text_input* TextInput,
	float FontHeight,
	char* Buffer,
	uint32_t BufferSize
)
{
	*TextInput = {};
	ClearAllFlags(TextInput);
	TextInput->UiId = GetId(UiContext);
	TextInput->CursorPos = 0;
	TextInput->FontHeight = FontHeight;
	TextInput->BufferSize = BufferSize;
	TextInput->Buffer = Buffer;
	TextInput->RepeatDelay = 1.0f;
	TextInput->RepeatPeriod = 0.05f;
	TextInput->CursorColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	TextInput->CursorAlphaState = CursorAlphaState_Decreasing;
}

void AddLetterToTextInput(text_input* TextInput)
{
	TextInput->Buffer[TextInput->CursorPos] = TextInput->CharDown;
	TextInput->CursorPos++;
	if(TextInput->CursorPos >= TextInput->BufferSize)
	{
		TextInput->CursorPos = TextInput->BufferSize - 1;
	}
	TextInput->Buffer[TextInput->CursorPos] = 0;
}

void Backspace(text_input* TextInput)
{
	if(TextInput->CursorPos > 0)
	{
		TextInput->CursorPos--;	
	}
	TextInput->Buffer[TextInput->CursorPos] = 0;
}

void PressAndHoldKeyboardEvent(
	text_input* TextInput,
	text_input_repeat_callback* RepeatCallback,
	game_keyboard_event* KeyboardEvent,
	char Character
)
{
	if(!KeyboardEvent->IsDown)
	{
		ClearFlag(TextInput, TextInput_CharDownDelay);
		ClearFlag(TextInput, TextInput_CharDown);

		TextInput->CursorColor.A = 1.0f;
		TextInput->CursorAlphaState = CursorAlphaState_Decreasing; 
	}
	else
	{
		TextInput->CursorColor.A = 1.0f;
		TextInput->CursorAlphaState = CursorAlphaState_Static;

		TextInput->CharDown = Character;
		TextInput->RepeatTimer = 0.0f;
		TextInput->RepeatCallback = RepeatCallback;
		RepeatCallback(TextInput);
		SetFlag(TextInput, TextInput_CharDownDelay);
	}
}

void TextInputHandleMouse(
	ui_context* UiContext,
	text_input* TextInput,
	rectangle Rectangle,
	game_mouse_event* MouseEvent,
	vector2 MouseEventWorldPos
)
{
	bool Inside = PointInRectangle(MouseEventWorldPos, Rectangle);
	ui_id Id = TextInput->UiId;
	if(IsActive(UiContext, Id))
	{
		if(MouseEvent->Type == PrimaryDown)
		{
			if(!IsHot(UiContext, Id))
			{
				ClearFlag(TextInput, TextInput_CharDownDelay);
				ClearFlag(TextInput, TextInput_CharDown);
				ClearFlag(TextInput, TextInput_ShiftIsDown);
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
}

text_input_kb_result TextInputHandleKeyboard(
	ui_context* UiContext,
	text_input* TextInput,
	game_keyboard_event* KeyboardEvent
)
{
	text_input_kb_result Result = TextInputKbResult_NoAction;
	if(!IsActive(UiContext, TextInput->UiId))
	{
		goto end;
	}
	
	switch(KeyboardEvent->Code)
	{
		// TODO: Handle backspace and delete
		case(0x08):
		{
			// NOTE: backspace
			PressAndHoldKeyboardEvent(
				TextInput, Backspace, KeyboardEvent, KeyboardEvent->Code
			);
			Result = TextInputKbResult_TextChanged;
			break;
		}
		case(0x09):
		{
			// NOTE: Tab
			// TODO: implement
			break;
		}
		case(0x10):
		{
			// NOTE: Shift
			// TODO: implement
			// TODO: handle someone pressing shift already when
			// CONT: selecting the text box
			if(KeyboardEvent->IsDown)
			{
				SetFlag(TextInput, TextInput_ShiftIsDown);
			}
			else
			{
				ClearFlag(TextInput, TextInput_ShiftIsDown);
			}
			break;
		}
		case(0xBE):
		{
			// NOTE: period
			PressAndHoldKeyboardEvent(
				TextInput,
				AddLetterToTextInput,
				KeyboardEvent,
				0x2e
			);
			Result = TextInputKbResult_TextChanged;
			break;
		}
		case(0x0D):
		{
			// NOTE: Return
			Result = TextInputKbResult_Submit;
			break;
		}
		case(0x20):
		{
			// NOTE: Space
			PressAndHoldKeyboardEvent(
				TextInput,
				AddLetterToTextInput,
				KeyboardEvent,
				KeyboardEvent->Code
			);
			Result = TextInputKbResult_TextChanged;
			break;
		}
		case(0x25):
		{
			// NOTE: Left
			// TODO: implement							
			break;
		}
		case(0x26):
		{
			// NOTE: Up
			// TODO: implement							
			break;
		}
		case(0x27):
		{
			// NOTE: Right
			// TODO: implement							
			break;
		}
		case(0x28):
		{
			// NOTE: Down
			// TODO: implement
			break;
		}
		case(0x30):
		case(0x31):
		case(0x32):
		case(0x33):
		case(0x34):
		case(0x35):
		case(0x36):
		case(0x37):
		case(0x38):
		case(0x39):
		{
			// NOTE: Numbers
			PressAndHoldKeyboardEvent(
				TextInput,
				AddLetterToTextInput,
				KeyboardEvent,
				KeyboardEvent->Code
			);
			Result = TextInputKbResult_TextChanged;
			break;
		}
		case(0x41):
		case(0x42):
		case(0x43):
		case(0x44):
		case(0x45):
		case(0x46):
		case(0x47):
		case(0x48):
		case(0x49):
		case(0x4A):
		case(0x4B):
		case(0x4C):
		case(0x4D):
		case(0x4E):
		case(0x4F):
		case(0x50):
		case(0x51):
		case(0x52):
		case(0x53):
		case(0x54):
		case(0x55):
		case(0x56):
		case(0x57):
		case(0x58):
		case(0x59):
		case(0x5A):
		case(0x5B):
		{
			// NOTE: Letters
			char Letter;
			if(!CheckFlag(TextInput, TextInput_ShiftIsDown))
			{
				Letter = KeyboardEvent->Code + 0x20; 
			}
			else
			{
				Letter = KeyboardEvent->Code;
			}
			PressAndHoldKeyboardEvent(
				TextInput, AddLetterToTextInput, KeyboardEvent, Letter
			);
			Result = TextInputKbResult_TextChanged;
			break;
		}
		case(0x7F):
		{
			// NOTE: Delete
			// TODO: implement
			break;
		}
	}
	goto end;

end:
	return Result;
}

text_input_update_result UpdateTextInput(
	ui_context* UiContext, text_input* TextInput, float DtForFrame
)
{
	text_input_update_result Result = TextInputUpdate_NoAction;
	if(!IsActive(UiContext, TextInput->UiId))
	{
		return Result;
	}

	float Period = 1.0f;
	if(TextInput->CursorAlphaState == CursorAlphaState_Increasing)
	{
		TextInput->CursorColor.A += DtForFrame / Period;
		if(TextInput->CursorColor.A >= 1.0f)
		{
			TextInput->CursorColor.A = 1.0f;
			TextInput->CursorAlphaState = CursorAlphaState_Decreasing;
		}
	}
	else if(TextInput->CursorAlphaState == CursorAlphaState_Decreasing)
	{
		TextInput->CursorColor.A -= DtForFrame / Period;
		if(TextInput->CursorColor.A <= 0.0f)
		{
			TextInput->CursorColor.A = 0.0f;
			TextInput->CursorAlphaState = CursorAlphaState_Increasing;
		}
	}

	if(CheckFlag(TextInput, TextInput_CharDownDelay))
	{
		if(TextInput->RepeatTimer >= TextInput->RepeatDelay)
		{
			TextInput->RepeatTimer = 0.0f;
			TextInput->RepeatCallback(TextInput);
			ClearFlag(TextInput, TextInput_CharDownDelay);
			SetFlag(TextInput, TextInput_CharDown);
			Result = TextInputUpdate_TextChanged;
		}
	}
	else if(CheckFlag(TextInput, TextInput_CharDown))
	{
		if(TextInput->RepeatTimer >= TextInput->RepeatPeriod)
		{
			TextInput->RepeatTimer = 0.0f;
			TextInput->RepeatCallback(TextInput);
			Result = TextInputUpdate_TextChanged;			
		}
	}
	TextInput->RepeatTimer += DtForFrame;

	return Result;
}

void PushCursor(
	ui_context* UiContext,
	text_input* TextInput,
	rectangle Rectangle,
	vector2 Offset,
	render_group* RenderGroup
)
{
	if(IsActive(UiContext, TextInput->UiId))
	{
		PushRect(
			RenderGroup,
			MakeRectangle(
				Rectangle.Min + Offset,
				Vector2(2.0f, TextInput->FontHeight)
			),
			TextInput->CursorColor
		);
	}
}

void PushTextInput(
	ui_context* UiContext,
	text_input* TextInput,
	vector4 FontColor,
	rectangle Rectangle,
	bitmap_handle Background,
	vector4 BackgroundColor,
	assets* Assets,
	render_group* RenderGroup,
	memory_arena* FrameArena
)
{
	PushSizedBitmap(
		RenderGroup,
		Assets,
		Background,
		GetCenter(Rectangle),
		Rectangle.Dim,				
		BackgroundColor
	);
	push_text_result PushTextResult = {};

	if(TextInput->Buffer[0] != 0)
	{		
		vector2 TopLeft = GetTopLeft(Rectangle);	
		PushTextResult = PushTextTopLeft(
			RenderGroup,
			Assets,
			FontHandle_TestFont,
			TextInput->Buffer,
			TextInput->BufferSize,
			TextInput->FontHeight,
			TopLeft,
			FontColor,
			FrameArena
		);
		if(PushTextResult.Code == PushText_Success)
		{
			vector2 OffsetScreen = PushTextResult.Offset;
			vector2 OffsetWorld = TransformVectorToBasis(
				RenderGroup->CameraToScreen, OffsetScreen
			);
			vector2 OffsetCamera = TransformVectorToBasis(
				RenderGroup->WorldToCamera, OffsetWorld
			);

			PushCursor(
				UiContext, TextInput, Rectangle, OffsetCamera, RenderGroup
			);
		}
	}
	if(PushTextResult.Code != PushText_Success)
	{
		// TODO: make the lpad more programmatic
		PushCursor(
			UiContext, TextInput, Rectangle, Vector2(2.0f, 0.0f), RenderGroup
		);
	}
}