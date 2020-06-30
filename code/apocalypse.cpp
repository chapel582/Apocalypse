#include "apocalypse.h"
#include "apocalypse_platform.h"
#include "apocalypse_intrinsics.h"

#include "apocalypse_render_group.h"
#include "apocalypse_render_group.cpp"

#include "apocalypse_bitmap.h"
#include "apocalypse_bitmap.cpp"

#include <math.h>
#include <stdlib.h>
#include <time.h>

#define PI32 3.14159265359f

// SECTION START: Temporary or Misc. code
void AlterData(void* Data)
{
	int* IntArray = (int*) Data;
	IntArray[0] = 123;
}
// SECTION END: Temporary or Misc. code

loaded_bitmap MakeEmptyBitmap(
	memory_arena* Arena, int32_t Width, int32_t Height
)
{
	void* Memory = PushSize(Arena, GetBitmapSize(Width, Height));
	return MakeEmptyBitmap(Memory, Width, Height);
}

bool AddCardToSet(card_set* CardSet, card* Card)
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

void AlignCardSet(card_set* CardSet)
{
	// NOTE: Basically, this is the amount of space not taken up by the 
	// CONT: cards divided by the number of spaces between and around the 
	// CONT: cards
	float Width = CardSet->CardWidth;
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
			Card->Rectangle.Min.X = CurrentXPos;
			Card->Rectangle.Min.Y = CardSet->YPos;
			CurrentXPos += DistanceBetweenCardPos;
		}
	}
}

void AddCardAndAlign(card_set* CardSet, card* Card)
{
	if(AddCardToSet(CardSet, Card))
	{
		AlignCardSet(CardSet);		
	}
}

bool RemoveCardFromSet(card_set* CardSet, card* Card)
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

void RemoveCardAndAlign(card_set* CardSet, card* Card)
{
	if(RemoveCardFromSet(CardSet, Card))
	{
		AlignCardSet(CardSet);
	}
}

void DrawFullHand(
	game_state* GameState, player_e Player, float CardWidth, float CardHeight
)
{
	deck* Deck = &GameState->Decks[Player];
	for(
		int CardIndex = 0;
		CardIndex < MAX_CARDS_PER_SET;
		CardIndex++
	)
	{
		card* Card = NULL;
		for(
			int SearchIndex = CardIndex;
			SearchIndex < GameState->MaxCards;
			SearchIndex++
		)
		{
			Card = &GameState->Cards[SearchIndex];
			if(!Card->Active)
			{
				break;
			}
		}
		ASSERT(Card != NULL);
		ASSERT(!Card->Active);

		Card->Rectangle.Dim.X = CardWidth;
		Card->Rectangle.Dim.Y = CardHeight;
		Card->TimeLeft = 10.0f;
		Card->Active = true;
		vector4* Color = &Card->Color;
		if(Player == Player_One)
		{
			Color->A = 1.0f;
			Color->R = 1.0f;
			Color->G = 0.0f;
			Color->B = 0.0f;
		}
		else
		{
			Color->A = 1.0f;
			Color->R = 0.0f;
			Color->G = 0.0f;
			Color->B = 1.0f;
		}
		
		deck_card* CardToDraw = Deck->OutOfDeck;
		ASSERT(CardToDraw != NULL);	
		Card->RedCost = CardToDraw->RedCost;
		Card->GreenCost = CardToDraw->GreenCost;
		Card->BlueCost = CardToDraw->BlueCost;
		Card->Owner = Player;
		AddCardToSet(&GameState->Hands[Player], Card);

		InDeckToOutDeck(Deck, CardToDraw);
		Card++;
	}
	AlignCardSet(&GameState->Hands[Player]);
}

#if APOCALYPSE_INTERNAL
game_memory* DebugGlobalMemory;
#endif 

void GameUpdateAndRender(
	game_memory* Memory,
	game_offscreen_buffer* BackBuffer,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
)
{
#if APOCALYPSE_INTERNAL
	DebugGlobalMemory = Memory;
#endif 

	BEGIN_TIMED_BLOCK(GameUpdateAndRender);

	ASSERT(sizeof(game_state) <= Memory->PermanentStorageSize);
	game_state* GameState = (game_state*) Memory->PermanentStorage;
	if(!Memory->IsInitialized)
	{
#if !APOCALYPSE_INTERNAL
		// NOTE: only init rand tools if we're in a release build
		srand(time(NULL));
#endif

		// NOTE: zero out memory at start just in case
		*GameState = {};
		// NOTE: right now, we assume everything after game state is just in the arena
		InitMemArena(
			&GameState->Arena,
			Memory->PermanentStorageSize - sizeof(game_state),
			((uint8_t*) Memory->PermanentStorage) + sizeof(game_state)
		);

		// TODO: do we want to be extra and make sure we pick up that last byte?
		size_t TransientStorageDivision = Memory->TransientStorageSize / 3;
		InitMemArena(
			&GameState->TransientArena,
			TransientStorageDivision,
			(uint8_t*) Memory->TransientStorage
		);
		InitMemArena(
			&GameState->RenderArena,
			TransientStorageDivision,
			GetEndOfArena(&GameState->TransientArena)
		);
		InitMemArena(
			&GameState->FrameArena,
			TransientStorageDivision,
			GetEndOfArena(&GameState->RenderArena)
		);

		GameState->WorldToCamera = MakeBasis(
			Vector2(BackBuffer->Width / 2.0f, BackBuffer->Height / 2.0f),
			Vector2(1.0f, 0.0f),
			Vector2(0.0f, 1.0f)
		);
		GameState->CameraToScreen = MakeBasis(
			Vector2(
				-1.0f * BackBuffer->Width / 2.0f,
				-1.0f * BackBuffer->Height / 2.0f
			),
			Vector2(1, 0),
			Vector2(0, 1)
		);

		render_group* RenderGroup = &GameState->RenderGroup;
		*RenderGroup = {};
		RenderGroup->Arena = &GameState->RenderArena;

		RenderGroup->WorldToCamera = &GameState->WorldToCamera;
		RenderGroup->CameraToScreen = &GameState->CameraToScreen;

		GameState->CurrentPrimaryState = PrimaryUp;
		GameState->Time = 0;
		GameState->ToneHz = 256;

		GameState->MaxCards = Player_Count * CardSet_Count * MAX_CARDS_PER_SET;
		GameState->Cards = PushArray(
			&GameState->Arena, GameState->MaxCards, card
		);
		{
			GameState->Decks = PushArray(&GameState->Arena, Player_Count, deck);
			for(
				int PlayerIndex = Player_One;
				PlayerIndex < Player_Count;
				PlayerIndex++
			)
			{
				deck* Deck = &GameState->Decks[PlayerIndex];
				*Deck = {};

				deck_card* DeckCard = &Deck->Cards[0];
				*DeckCard = {};
				DeckCard->RedCost = 0;
				DeckCard->GreenCost = 0;
				DeckCard->BlueCost = 0;
				DeckCard->Next = NULL;
				DeckCard->Previous = NULL;
				Deck->OutOfDeck = DeckCard;
				Deck->OutOfDeckLength++;
				for(
					int CardIndex = 1;
					CardIndex < MAX_CARDS_IN_DECK;
					CardIndex++
				)
				{
					deck_card* PrevDeckCard = &Deck->Cards[CardIndex - 1];
					DeckCard = &Deck->Cards[CardIndex];
					*DeckCard = {};
					DeckCard->RedCost = CardIndex;
					DeckCard->GreenCost = 2 * CardIndex;
					DeckCard->BlueCost = 3 * CardIndex;
					DeckCard->Next = NULL;
					DeckCard->Previous = PrevDeckCard;
					PrevDeckCard->Next = DeckCard;
					Deck->OutOfDeckLength++;
				}
				DeckCard->Next = Deck->OutOfDeck;
			}

			// SECTION START: Shuffle deck
			for(
				int PlayerIndex = Player_One;
				PlayerIndex < Player_Count;
				PlayerIndex++
			)
			{
				deck* Deck = &GameState->Decks[PlayerIndex];

				while(Deck->OutOfDeckLength > 0)
				{
					int DeckCardIndex = rand() % Deck->OutOfDeckLength;
					deck_card* CardToShuffle = GetDeckCard(
						Deck->OutOfDeck, Deck->OutOfDeckLength, DeckCardIndex
					);
					OutDeckToInDeck(Deck, CardToShuffle);
				}

				deck_card* DeckCard = &Deck->Cards[0];
				for(
					int CardIndex = 1;
					CardIndex < MAX_CARDS_IN_DECK;
					CardIndex++
				)
				{
					deck_card* PrevDeckCard = &Deck->Cards[CardIndex - 1];
					DeckCard = &Deck->Cards[CardIndex];
					*DeckCard = {};
					DeckCard->RedCost = CardIndex;
					DeckCard->GreenCost = 2 * CardIndex;
					DeckCard->BlueCost = 3 * CardIndex;
					DeckCard->Next = NULL;
					DeckCard->Previous = PrevDeckCard;
					PrevDeckCard->Next = DeckCard;
					Deck->OutOfDeckLength++;
				}
				DeckCard->Next = Deck->OutOfDeck;
			}
			// SECTION STOP: Shuffle deck
		}

		GameState->Hands = PushArray(&GameState->Arena, Player_Count, card_set);
		GameState->Tableaus = PushArray(
			&GameState->Arena, Player_Count, card_set
		);

		float CardWidth = 60.0f;
		float CardHeight = 90.0f;
		float HandTableauMargin = 5.0f;
		// NOTE: transform assumes screen and camera are 1:1
		vector2 ScreenDimInWorld = TransformVectorToBasis(
			&GameState->WorldToCamera,
			Vector2(BackBuffer->Width, BackBuffer->Height)
		);
		float ScreenWidthInWorld = ScreenDimInWorld.X;
		
		card_set* CardSet = &GameState->Hands[Player_One];
		CardSet->CardCount = 0;
		CardSet->ScreenWidth = ScreenWidthInWorld;
		CardSet->YPos = 0.0f;
		CardSet->CardWidth = CardWidth;

		CardSet = &GameState->Hands[Player_Two];
		CardSet->CardCount = 0;
		CardSet->ScreenWidth = ScreenWidthInWorld;
		CardSet->YPos = ScreenDimInWorld.Y - CardHeight;
		CardSet->CardWidth = CardWidth;

		CardSet = &GameState->Tableaus[Player_One];
		CardSet->CardCount = 0;
		CardSet->ScreenWidth = ScreenWidthInWorld;
		CardSet->YPos = CardHeight + HandTableauMargin;
		CardSet->CardWidth = CardWidth;

		CardSet = &GameState->Tableaus[Player_Two];
		CardSet->CardCount = 0;
		CardSet->ScreenWidth = ScreenWidthInWorld;
		CardSet->YPos = (
			ScreenDimInWorld.Y - (2 * CardHeight) - HandTableauMargin 
		);
		CardSet->CardWidth = CardWidth;
		
		DrawFullHand(GameState, Player_One, CardWidth, CardHeight);
		DrawFullHand(GameState, Player_Two, CardWidth, CardHeight);

		{
			GameState->TestCard = DEBUGLoadBmp("../data/test/test_card.bmp");
			GameState->TestBitmap = DEBUGLoadBmp("../data/test/tree00.bmp");
			GameState->TestBackground = DEBUGLoadBmp(
				"../data/test/test_background.bmp"
			);


			GameState->EnvMapWidth = 512;
			GameState->EnvMapHeight = 256;
			for(
				uint32_t MapIndex = 0;
				MapIndex < ARRAY_COUNT(GameState->EnvMaps);
				MapIndex++
			)
			{
				environment_map* Map = GameState->EnvMaps + MapIndex;
				uint32_t Width = GameState->EnvMapWidth;
				uint32_t Height = GameState->EnvMapHeight;
				for(
					uint32_t LodIndex = 0;
					LodIndex < ARRAY_COUNT(Map->Lod);
					LodIndex++
				)
				{
					Map->Lod[LodIndex] = MakeEmptyBitmap(
						&GameState->TransientArena, Width, Height
					);
					Width >>= 1;
					Height >>= 1;
				}
			}
		}

		Memory->IsInitialized = true;
	}

	GameState->Time += DtForFrame;

#if 0
	// TODO: delete this when we're using the job queue for other things
	int IntArray1[] = {0, 1, 2, 3};
	PlatformAddJob(Memory->DefaultJobQueue, AlterData, &IntArray1);
	int IntArray2[] = {0, 1, 2, 4};
	PlatformAddJob(Memory->DefaultJobQueue, AlterData, &IntArray2);
	int IntArray3[] = {0, 1, 2, 3};
	PlatformAddJob(Memory->DefaultJobQueue, AlterData, &IntArray3);
	int IntArray4[] = {0, 1, 2, 3};
	PlatformAddJob(Memory->DefaultJobQueue, AlterData, &IntArray4);
	int IntArray5[] = {0, 1, 2, 3};
	PlatformAddJob(Memory->DefaultJobQueue, AlterData, &IntArray5);
	int IntArray6[] = {0, 1, 2, 3};
	PlatformAddJob(Memory->DefaultJobQueue, AlterData, &IntArray6);
	int IntArray7[] = {0, 1, 2, 3};
	PlatformAddJob(Memory->DefaultJobQueue, AlterData, &IntArray7);
	int IntArray8[] = {0, 1, 2, 3};
	PlatformAddJob(Memory->DefaultJobQueue, AlterData, &IntArray8);
	int IntArray9[] = {0, 1, 2, 3};
	PlatformAddJob(Memory->DefaultJobQueue, AlterData, &IntArray9);
	int IntArray10[] = {0, 1, 2, 3};
	PlatformAddJob(Memory->DefaultJobQueue, AlterData, &IntArray10);
	int IntArray11[] = {0, 1, 2, 3};
	PlatformAddJob(Memory->DefaultJobQueue, AlterData, &IntArray11);
	int IntArray12[] = {0, 1, 2, 3};
	PlatformAddJob(Memory->DefaultJobQueue, AlterData, &IntArray12);
	int IntArray13[] = {0, 1, 2, 3};
	PlatformAddJob(Memory->DefaultJobQueue, AlterData, &IntArray13);
	int IntArray14[] = {0, 1, 2, 3};
	PlatformAddJob(Memory->DefaultJobQueue, AlterData, &IntArray14);
	int IntArray15[] = {0, 1, 2, 3};
	PlatformAddJob(Memory->DefaultJobQueue, AlterData, &IntArray15);
	int IntArray16[] = {0, 1, 2, 3};
	PlatformAddJob(Memory->DefaultJobQueue, AlterData, &IntArray16);
	PlatformCompleteAllJobs(Memory->DefaultJobQueue);
#endif 

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
			game_mouse_event* MouseEvent = (
				&MouseEvents->Events[MouseEventIndex]
			);

			if(MouseEvent->UserEventIndex != UserEventIndex)
			{
				break;
			}

			vector2 MouseEventWorldPos = TransformPosFromBasis(
				&GameState->WorldToCamera,
				TransformPosFromBasis(
					&GameState->CameraToScreen, 
					Vector2(MouseEvent->XPos, MouseEvent->YPos)
				)
			);
			if(MouseEvent->Type == PrimaryUp)
			{
				card* Card = &GameState->Cards[0];
				for(
					int CardIndex = 0;
					CardIndex < GameState->MaxCards;
					CardIndex++
				)
				{
					if(
						Card->Active &&
						PointInRectangle(MouseEventWorldPos, Card->Rectangle)
					)
					{
						RemoveCardAndAlign(
							&GameState->Hands[Card->Owner], Card
						);
						AddCardAndAlign(
							&GameState->Tableaus[Card->Owner], Card
						);
						break;
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
				}
			}

			UserEventIndex++;
		}
	}
	// SECTION STOP: User input

	// SECTION START: Updating game state
	// NOTE: not sure if we should finish all updates and then push to the 
	// CONT: render group
	// TODO: remove scaling test code
	// float ScaleValue = cosf((2 * PI32 / 5.0f) * GameState->Time);
	// GameState->WorldToCamera = MakeBasis(
	// 	Vector2(BackBuffer->Width / 2.0f, BackBuffer->Height / 2.0f),
	// 	ScaleValue * Vector2(1.0f, 0.0f),
	// 	ScaleValue * Vector2(0.0f, 1.0f)
	// );
	PushClear(&GameState->RenderGroup, Vector4(0.25f, 0.25f, 0.25f, 1.0f));
	PushSizedBitmap(
		&GameState->RenderGroup,
		&GameState->TestBackground,
		Vector2((BackBuffer->Width / 2.0f), (BackBuffer->Height / 2.0f)),
		Vector2(BackBuffer->Width, 0),
		Vector2(0, BackBuffer->Height),
		Vector4(1.0f, 1.0f, 1.0f, 1.0f),
		NULL,
		NULL,
		NULL,
		NULL
	);
	{
		card* Card = &GameState->Cards[0];
		for(
			int CardIndex = 0;
			CardIndex < GameState->MaxCards;
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
				PushSizedBitmap(
					&GameState->RenderGroup,
					&GameState->TestCard,
					GetCenter(Card->Rectangle),
					Vector2(Card->Rectangle.Dim.X, 0.0f),
					Vector2(0.0f, Card->Rectangle.Dim.Y),
					Card->Color,
					NULL,
					NULL,
					NULL,
					NULL
				);
			}
			Card++;
		}
	}

#if 1 // NOTE: tests for bitmaps
	float RotationalPeriod = 2.0f;
	float Radians = (2 * PI32 * GameState->Time) / RotationalPeriod;
	float CosVal = cosf(Radians);
	float SinVal = sinf(Radians);
	vector2 Origin = Vector2(0.0f, BackBuffer->Height / 2.0f);
	vector2 Center = Vector2(
		(BackBuffer->Width / 2.0f), (BackBuffer->Height / 2.0f)
	);
	vector2 XAxis = Vector2(CosVal, SinVal);
	vector2 YAxis = Perpendicular(XAxis);

	PushCenteredBitmap(
		&GameState->RenderGroup,
		&GameState->TestBitmap,
		Center,
		XAxis,
		YAxis,
		Vector4(1.0f, 1.0f, 1.0f, 1.0f),
		NULL,
		NULL,
		NULL,
		NULL
	);

	basis RectBasis = MakeBasis(Vector2(0, 0), Vector2(1, 0), Vector2(0, 1));
	PushRect(
		&GameState->RenderGroup,
		&RectBasis,
		MakeRectangle(Vector2(0, 0), Vector2(25, 25)),
		Vector4(1.0f, 1.0f, 1.0f, 1.0f)
	);
#endif

	// SECTION STOP: Updating game state

	// SECTION START: Render
	loaded_bitmap DrawBuffer = {};
	DrawBuffer.Width = BackBuffer->Width;
	DrawBuffer.Height = BackBuffer->Height;
	DrawBuffer.Pitch = BackBuffer->Pitch;
	DrawBuffer.Memory = BackBuffer->Memory;

	RenderGroupToOutput(&GameState->RenderGroup, &DrawBuffer);
	// SECTION STOP: Render
	ResetMemArena(&GameState->FrameArena);

	END_TIMED_BLOCK(GameUpdateAndRender);
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
		GameState->SineT += (2.0f * PI32 * 1.0f) / ((float) WavePeriod);

		// NOTE: SampleOut writes left and right channels
		*SampleOut++ = SampleValue;
		*SampleOut++ = SampleValue;
	}
#endif
}