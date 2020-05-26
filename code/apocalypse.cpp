#include "apocalypse.h"

#include <math.h>

#define Pi32 3.14159265359f

int32_t RoundFloat32ToInt32(float Input)
{
	return (int32_t) (Input + 0.5f);
}

void DrawRectangle(
	game_offscreen_buffer* BackBuffer, 
	float MinXF,
	float MinYF,
	float MaxXF,
	float MaxYF,
	float Red, 
	float Green, 
	float Blue
)
{
	int32_t MinX = RoundFloat32ToInt32(MinXF);
	int32_t MinY = RoundFloat32ToInt32(MinYF);
	int32_t MaxX = RoundFloat32ToInt32(MaxXF);
	int32_t MaxY = RoundFloat32ToInt32(MaxYF);

	if(MinX < 0)
	{
		MinX = 0;
	}
	else if(MinX > BackBuffer->Width)
	{
		MinX = BackBuffer->Width;
	}
	if(MaxX < 0)
	{
		MaxX = 0;
	}
	else if(MaxX > BackBuffer->Width)
	{
		MaxX = BackBuffer->Width;
	}
	if(MinY < 0)
	{
		MinY = 0;
	}
	else if(MinY > BackBuffer->Height)
	{
		MinY = BackBuffer->Height;
	}

	uint32_t Color = (
		(RoundFloat32ToInt32(Red * 0xFF) << 16) |
		(RoundFloat32ToInt32(Green * 0xFF) << 8) |
		RoundFloat32ToInt32(Blue * 0xFF)
	);

	uint8_t* Row = (
		((uint8_t*) BackBuffer->Memory) + 
		MinX * BackBuffer->BytesPerPixel +
		MinY * BackBuffer->Pitch 
	);

	for(int Y = MinY; Y < MaxY; Y++)
	{
		uint32_t* Pixel = (uint32_t*) Row; 
		for(int X = MinX; X < MaxX; X++)
		{
			*Pixel++ = Color;
		}
		Row += BackBuffer->Pitch;
	}
}

void GameUpdateAndRender(
	game_memory* Memory,
	game_offscreen_buffer* BackBuffer,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float dtForFrame
)
{
	ASSERT(sizeof(game_state) <= Memory->PermanentStorageSize);
	game_state* GameState = (game_state*) Memory->PermanentStorage;
	if(!Memory->IsInitialized)
	{
		char* FileName = __FILE__;
		debug_read_file_result File = DEBUGPlatformReadEntireFile(FileName);
		if(File.Contents)
		{
			DEBUGPlatformWriteEntireFile(
				"test.out", File.Contents, File.ContentsSize
			);
			DEBUGPlatformFreeFileMemory(File.Contents);
		}

		GameState->XOffset = 0;
		GameState->YOffset = 0;
		GameState->CurrentPrimaryState = PrimaryUp;
		GameState->SineT = 0;
		GameState->ToneHz = 256;
		GameState->TempBuffer[1024];
		GameState->TempBufferLength = 0;

		float Width = 30.0f;
		float Height = 30.0f;
		float XMargin = 10.0f;
		float CardSpacing = (
			(BackBuffer->Width - 2 * XMargin) / 
			(ARRAY_COUNT(GameState->Cards) / 2)
		);
		float CurrentXPos = XMargin;
		card* Card = &GameState->Cards[0];
		int CardIndex;
		for(
			CardIndex = 0;
			CardIndex < ARRAY_COUNT(GameState->Cards) / 2;
			CardIndex++
		)
		{
			Card->PosX = CurrentXPos;
			Card->PosY = 0.0f;
			Card->Width = Width;
			Card->Height = Height;
			Card->TimeLeft = 10.0f;
			Card->Active = true;
			CurrentXPos += CardSpacing;
			Card++;
		}
		CurrentXPos = XMargin;
		float LowerYPos = BackBuffer->Height - Height;
		for(
			;
			CardIndex < ARRAY_COUNT(GameState->Cards);
			CardIndex++
		)
		{
			Card->PosX = CurrentXPos;
			Card->PosY = LowerYPos;
			Card->Width = Width;
			Card->Height = Height;
			Card->TimeLeft = 10.0f;
			Card->Active = true;
			CurrentXPos += CardSpacing;
			Card++;
		}

		// TODO: this may be more appropriate in the platform layer
		Memory->IsInitialized = true;
	}

	// SECTION START: User input
	// TODO: move to game memory
	user_event_index UserEventIndex = 0;
	int MouseEventIndex = 0;
	int KeyboardEventIndex = 0;
	while(
		(MouseEventIndex < MouseEvents->Length) ||
		(KeyboardEventIndex < KeyboardEvents->Length)
	)
	{
		for(; MouseEventIndex < MouseEvents->Length; MouseEventIndex++)
		{
			// TODO: remove last action wins stuff from 
			// NOTE: testing platform layer mouse stuff
			game_mouse_event* MouseEvent = &MouseEvents->Events[MouseEventIndex];

			if(MouseEvent->UserEventIndex != UserEventIndex)
			{
				break;
			}

			if(
				MouseEvent->Type == PrimaryDown ||
				MouseEvent->Type == PrimaryUp
			)
			{
				GameState->CurrentPrimaryState = MouseEvent->Type;
			}
			if(GameState->CurrentPrimaryState == PrimaryDown)
			{
				GameState->XOffset = BackBuffer->Width - MouseEvent->XPos;
				GameState->YOffset = BackBuffer->Height - MouseEvent->YPos;
			}

			UserEventIndex++;
		}

		for(; KeyboardEventIndex < KeyboardEvents->Length; KeyboardEventIndex++)
		{
			game_keyboard_event* KeyboardEvent = (
				&KeyboardEvents->Events[KeyboardEventIndex]
			);
			if(KeyboardEvent->UserEventIndex != UserEventIndex)
			{
				break;
			}

			if(KeyboardEvent->Code >= 0x41 && KeyboardEvent->Code <= 0x5A)
			{
				if(KeyboardEvent->IsDown != KeyboardEvent->WasDown)
				{
					GameState->TempBuffer[GameState->TempBufferLength++] = (
						KeyboardEvent->Code
					);
				}
			}

			UserEventIndex++;
		}
	}

	if(GameState->TempBufferLength >= 12)
	{
		DEBUGPlatformWriteEntireFile(
			"keyboardtest.out", GameState->TempBuffer, GameState->TempBufferLength
		);
		GameState->TempBufferLength = 0;
	}
	// SECTION STOP: User input

	// SECTION START: Render
	DrawRectangle(
		BackBuffer,
		0.0f,
		0.0f,
		(float) BackBuffer->Width,
		(float) BackBuffer->Height,
		1.0f,
		0.0f,
		0.0f
	);

	card* Card = &GameState->Cards[0];
	for(
		int CardIndex = 0;
		CardIndex < ARRAY_COUNT(GameState->Cards);
		CardIndex++
	)
	{
		DrawRectangle(BackBuffer, Card->PosX, Card->PosY, Card->PosX + Card->Width, Card->PosY + Card->Height, 1.0f, 1.0f, 1.0f);
		Card++;
	}
#if 0
	uint8_t* Row = (uint8_t*) BackBuffer->Memory;
	for(int Y = 0; Y < BackBuffer->Height; Y++)
	{
		uint32_t* Pixel = (uint32_t*) Row;
		for(int X = 0; X < BackBuffer->Width; X++)
		{
			// *Pixel = 0x000000;
			uint8_t* ColorChannel = (uint8_t*) Pixel;
			*ColorChannel++ = (uint8_t) (X + GameState->XOffset);
			*ColorChannel++ = (uint8_t) (Y + GameState->YOffset);
			*ColorChannel++ = 0;
			Pixel++;
		}
		Row += BackBuffer->Pitch;
	}
#endif
	// SECTION STOP: Render
}

void GameFillSound(game_memory* Memory, game_sound_output_buffer* SoundBuffer)
{
	ASSERT(sizeof(game_state) <= Memory->PermanentStorageSize);
	game_state* GameState = (game_state*) Memory->PermanentStorage;
#if 0
	int16_t ToneVolume = 3000;
	int WavePeriod = SoundBuffer->SamplesPerSecond / GameState->ToneHz;
	int16_t* SampleOut = SoundBuffer->Samples;
	for(
		int SampleIndex = 0;
		SampleIndex < SoundBuffer->SampleCount;
		++SampleIndex
	)
	{
		float SineValue = sinf(GameState->SineT);
		int16_t SampleValue = (int16_t) (SineValue * ToneVolume);
		GameState->SineT += (2.0f * Pi32 * 1.0f) / ((float) WavePeriod);

		// NOTE: SampleOut writes left and right channels
		*SampleOut++ = SampleValue;
		*SampleOut++ = SampleValue;
	}
#endif
}