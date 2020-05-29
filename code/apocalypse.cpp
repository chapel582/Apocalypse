#include "apocalypse.h"

#include <math.h>

#define Pi32 3.14159265359f

int32_t RoundFloat32ToInt32(float Input)
{
	return (int32_t) (Input + 0.5f);
}

vector2 ScreenToWorldPos(
	world_screen_converter Converter, uint32_t X, uint32_t Y
)
{
	vector2 Result = {};
	Result.X = Converter.ScreenToWorld * X;
	Result.Y = (
		(-1.0f * Converter.ScreenToWorld * Y) + Converter.WorldYOffset
	);
	return Result;
}

vector2 WorldToScreenPos(
	world_screen_converter Converter, vector2 WorldPosition
)
{
	vector2 Result = {};
	Result.X = Converter.WorldToScreen * WorldPosition.X;
	Result.Y = (
		(-1.0f * Converter.WorldToScreen * WorldPosition.Y) + Converter.ScreenYOffset
	);
	return Result;
}

vector2 ScreenToWorldDim(world_screen_converter Converter, vector2 ScreenDim)
{
	vector2 Result = {};
	Result.X = Converter.ScreenToWorld * ScreenDim.X;
	Result.Y = Converter.ScreenToWorld * ScreenDim.Y;
	return Result;
}

vector2 WorldToScreenDim(world_screen_converter Converter, vector2 WorldDim)
{
	vector2 Result = {};
	Result.X = Converter.WorldToScreen * WorldDim.X;
	Result.Y = Converter.WorldToScreen * WorldDim.Y;
	return Result;
}

bool PointInRectangle(
	vector2 Point, float MinX, float MinY, float MaxX, float MaxY
)
{
	return (
		(Point.X >= MinX && Point.X < MaxX) && 
		(Point.Y >= MinY && Point.Y < MaxY)
	);
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
	if(MaxY < 0)
	{
		MaxY = 0;
	}
	else if(MaxY > BackBuffer->Height)
	{
		MaxY = BackBuffer->Height;
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

bool AddCardToSet(card_set_s* CardSet, card* Card)
{
	for(int Index = 0; Index < ARRAY_COUNT(CardSet->Cards); Index++)
	{
		if(CardSet->Cards[Index] == Card)
		{
			return false;
		}
		else if(CardSet->Cards[Index] == NULL)
		{
			CardSet->Cards[Index] = Card;
			CardSet->CardCount++;
			return true;
		}
	}
	ASSERT(false);
	return false;
}

void AlignCardSet(card_set_s* CardSet)
{
	// NOTE: Basically, this is the amount of space not taken up by the 
	// CONT: cards divided by the number of spaces between and around the 
	// CONT: cards
	float Width = CardSet->Cards[0]->Dim.X;
	float SpaceSize = (
		(CardSet->ScreenWidth - (CardSet->CardCount * Width)) / 
		(CardSet->CardCount + 1)
	);
	float DistanceBetweenCardPos = SpaceSize + Width;
	float CurrentXPos = SpaceSize;
	for(int Index = 0; Index < ARRAY_COUNT(CardSet->Cards); Index++)
	{
		card* Card = CardSet->Cards[Index];
		if(Card != NULL && Card->Active)
		{
			Card->Pos.X = CurrentXPos;
			Card->Pos.Y = CardSet->YPos;
			CurrentXPos += DistanceBetweenCardPos;
		}
	}
}

void AddCardAndAlign(card_set_s* CardSet, card* Card)
{
	if(AddCardToSet(CardSet, Card))
	{
		AlignCardSet(CardSet);		
	}
}

bool RemoveCardFromSet(card_set_s* CardSet, card* Card)
{
	for(int Index = 0; Index < ARRAY_COUNT(CardSet->Cards); Index++)
	{
		if(CardSet->Cards[Index] == Card)
		{
			CardSet->Cards[Index] = NULL;
			CardSet->CardCount--;
			return true;
		}
	}
	return false;
}

void RemoveCardAndAlign(card_set_s* CardSet, card* Card)
{
	if(RemoveCardFromSet(CardSet, Card))
	{
		AlignCardSet(CardSet);
	}
}

void GameUpdateAndRender(
	game_memory* Memory,
	game_offscreen_buffer* BackBuffer,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
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
		// NOTE: zero out memory at start just in case
		*GameState = {};

		GameState->XOffset = 0;
		GameState->YOffset = 0;
		GameState->CurrentPrimaryState = PrimaryUp;
		GameState->SineT = 0;
		GameState->ToneHz = 256;
		GameState->TempBuffer[1024];
		GameState->TempBufferLength = 0;

		// NOTE: right now, the conversion assumes that the screen origin and 
		// CONT: the world origin are on the same place on the x axis
		world_screen_converter* WorldScreenConverter = (
			&GameState->WorldScreenConverter
		);
		WorldScreenConverter->ScreenToWorld = 1.0f;
		WorldScreenConverter->WorldToScreen = (
			1.0f / WorldScreenConverter->ScreenToWorld
		);
		WorldScreenConverter->ScreenYOffset = (float) BackBuffer->Height;
		WorldScreenConverter->WorldYOffset = (
			WorldScreenConverter->ScreenToWorld * 
			WorldScreenConverter->ScreenYOffset
		);

		float CardWidth = 60.0f;
		float CardHeight = 90.0f;
		float HandTableauMargin = 5.0f;
		float ScreenWidthInWorld = (
			WorldScreenConverter->ScreenToWorld * BackBuffer->Width
		);
		card_set_s* CardSet = &GameState->Hands[Player_One];
		CardSet->CardCount = 0;
		CardSet->ScreenWidth = ScreenWidthInWorld;
		CardSet->YPos = 0.0f;
		CardSet = &GameState->Hands[Player_Two];
		CardSet->CardCount = 0;
		CardSet->ScreenWidth = ScreenWidthInWorld;
		CardSet->YPos = (
			(WorldScreenConverter->ScreenToWorld * BackBuffer->Height) - 
			CardHeight 
		);
		CardSet = &GameState->Tableaus[Player_One];
		CardSet->CardCount = 0;
		CardSet->ScreenWidth = ScreenWidthInWorld;
		CardSet->YPos = CardHeight + HandTableauMargin;
		CardSet = &GameState->Tableaus[Player_Two];
		CardSet->CardCount = 0;
		CardSet->ScreenWidth = ScreenWidthInWorld;
		CardSet->YPos = (
			(WorldScreenConverter->ScreenToWorld * BackBuffer->Height) - 
			(2 * CardHeight) -
			HandTableauMargin 
		);
		
		card* Card = &GameState->Cards[0];
		int CardIndex;
		for(
			CardIndex = 0;
			CardIndex < MAX_CARDS_PER_SET;
			CardIndex++
		)
		{
			Card->Dim.X = CardWidth;
			Card->Dim.Y = CardHeight;
			Card->TimeLeft = 10.0f;
			Card->Active = true;
			Card->Red = 1.0f;
			Card->Green = 1.0f;
			Card->Blue = 1.0f;
			Card->Owner = Player_One;
			AddCardToSet(&GameState->Hands[Player_One], Card);
			Card++;
		}
		AlignCardSet(&GameState->Hands[Player_One]);

		for(
			CardIndex = 0;
			CardIndex < MAX_CARDS_PER_SET;
			CardIndex++
		)
		{
			Card->Dim.X = CardWidth;
			Card->Dim.Y = CardHeight;
			Card->TimeLeft = 10.0f;
			Card->Active = true;
			Card->Red = 1.0f;
			Card->Green = 1.0f;
			Card->Blue = 1.0f;
			Card->Owner = Player_Two;
			AddCardToSet(&GameState->Hands[Player_Two], Card);
			Card++;
		}
		AlignCardSet(&GameState->Hands[Player_Two]);

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
#if 0
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
#endif
			vector2 MouseEventWorldPos = ScreenToWorldPos(
				GameState->WorldScreenConverter,
				MouseEvent->XPos,
				MouseEvent->YPos
			);
			if(MouseEvent->Type == PrimaryUp)
			{
				card* Card = &GameState->Cards[0];
				for(
					int CardIndex = 0;
					CardIndex < ARRAY_COUNT(GameState->Cards);
					CardIndex++
				)
				{
					if(
						Card->Active &&
						PointInRectangle(
							MouseEventWorldPos,
							Card->Pos.X,
							Card->Pos.Y,
							Card->Pos.X + Card->Dim.X,
							Card->Pos.Y + Card->Dim.Y
						)
					)
					{
						RemoveCardAndAlign(
							&GameState->Hands[Card->Owner], Card
						);
						AddCardAndAlign(
							&GameState->Tableaus[Card->Owner], Card
						);
					}
					Card++;
				}
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
	// SECTION STOP: User input

	// SECTION START: Updating game state
	{
		card* Card = &GameState->Cards[0];
		for(
			int CardIndex = 0;
			CardIndex < ARRAY_COUNT(GameState->Cards);
			CardIndex++
		)
		{
			if(Card->Active)
			{
				Card->TimeLeft -= DtForFrame;
				if(Card->TimeLeft <= 0)
				{
					Card->Active = false;
				}
			}
			Card++;
		}
	}
	// SECTION STOP: Updating game state

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
		if(Card->Active)
		{
			vector2 WorldTopLeft = Vector2(
				Card->Pos.X, Card->Pos.Y + Card->Dim.Y
			);
			vector2 ScreenPos = WorldToScreenPos(
				GameState->WorldScreenConverter, WorldTopLeft
			);
			vector2 ScreenDim = WorldToScreenDim(
				GameState->WorldScreenConverter, Card->Dim
			);
			DrawRectangle(
				BackBuffer,
				ScreenPos.X,
				ScreenPos.Y,
				ScreenPos.X + ScreenDim.X,
				ScreenPos.Y + ScreenDim.Y,
				Card->Red,
				Card->Green,
				Card->Blue
			);
		}
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