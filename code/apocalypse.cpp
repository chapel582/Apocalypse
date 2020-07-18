#include "apocalypse.h"
#include "apocalypse_platform.h"
#include "apocalypse_intrinsics.h"
#include "apocalypse_debug.h"
#include "apocalypse_string.h"

#include "apocalypse_render_group.h"
#include "apocalypse_render_group.cpp"

#include "apocalypse_bitmap.h"
#include "apocalypse_bitmap.cpp"

#include "apocalypse_wav.h"
#include "apocalypse_wav.cpp"

#include "apocalypse_assets.h"
#include "apocalypse_assets.cpp"

#include "apocalypse_audio.h"
#include "apocalypse_audio.cpp"

#include "apocalypse_particles.h"
#include "apocalypse_particles.cpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

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
	Card->SetType = CardSet->Type;
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
	game_state* GameState, player_id Player, float CardWidth, float CardHeight
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
		for(
			int PlayerIndex = 0;
			PlayerIndex < Player_Count;
			PlayerIndex++
		)
		{
			Card->PlayDelta[PlayerIndex] = CardToDraw->PlayDelta[PlayerIndex];
			Card->TapDelta[PlayerIndex] = CardToDraw->TapDelta[PlayerIndex];
		}
		Card->Owner = Player;
		AddCardToSet(&GameState->Hands[Player], Card);

		InDeckToOutDeck(Deck, CardToDraw);
		Card++;
	}
	AlignCardSet(&GameState->Hands[Player]);
}

void GameUpdateAndRender(
	game_memory* Memory,
	game_offscreen_buffer* BackBuffer,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
)
{
	TIMED_BLOCK();

	ASSERT(sizeof(game_state) <= Memory->PermanentStorageSize);
	game_state* GameState = (game_state*) Memory->PermanentStorage;
	if(!Memory->IsInitialized)
	{
#if APOCALYPSE_RELEASE
		// TODO: replace with bespoke random tool
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
		memory_arena AssetArena;
		size_t TransientStorageDivision = Memory->TransientStorageSize / 4;
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
		InitMemArena(
			&AssetArena,
			TransientStorageDivision,
			GetEndOfArena(&GameState->FrameArena)
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

		GameState->Time = 0;

		assets* Assets = &GameState->Assets; 
		*Assets = {}; 
		Assets->Arena = AssetArena;
		Assets->JobQueue = Memory->DefaultJobQueue;
		Assets->ArenaLock = PlatformCreateMutex();
		Assets->AvailableListLock = PlatformCreateMutex();
		Assets->AvailableHead = NULL;
		for(
			int InfoIndex = 0;
			InfoIndex < ARRAY_COUNT(Assets->BitmapInfo);
			InfoIndex++
		)
		{
			asset_info* AssetInfo = &Assets->BitmapInfo[InfoIndex];
			AssetInfo->State = AssetState_Unloaded;
		}
		for(
			int InfoIndex = 0;
			InfoIndex < ARRAY_COUNT(Assets->WavInfo);
			InfoIndex++
		)
		{
			asset_info* AssetInfo = &Assets->WavInfo[InfoIndex];
			AssetInfo->State = AssetState_Unloaded;
		}
		for(
			int InfoIndex = 0;
			InfoIndex < ARRAY_COUNT(Assets->FontInfo);
			InfoIndex++
		)
		{
			asset_info* AssetInfo = &Assets->FontInfo[InfoIndex];
			AssetInfo->State = AssetState_Unloaded;
		}
		for(int FontHandle = 0; FontHandle < FontHandle_Count; FontHandle++)
		{
			for(
				int InfoIndex = 0;
				InfoIndex < ARRAY_COUNT(Assets->GlyphInfo);
				InfoIndex++
			)
			{
				asset_info* AssetInfo = (
					&Assets->GlyphInfo[FontHandle][InfoIndex]
				);
				AssetInfo->State = AssetState_Unloaded;
			}
		}

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
				for(
					int DeltaIndex = 0;
					DeltaIndex < Player_Count;
					DeltaIndex++
				)
				{
					player_resources* ResourceDelta = (
						&DeckCard->PlayDelta[DeltaIndex]
					);
					ResourceDelta->Red = -1;
					ResourceDelta->Green = 1;
					ResourceDelta->Blue = (rand() % 10) - 5;
					ResourceDelta->White = (rand() % 10) - 5;
					ResourceDelta->Black = (rand() % 10) - 5;

					ResourceDelta = &DeckCard->TapDelta[DeltaIndex];
					ResourceDelta->Red = 1;
					ResourceDelta->Green = -1;
					ResourceDelta->Blue = (rand() % 10) - 5;
					ResourceDelta->White = (rand() % 10) - 5;
					ResourceDelta->Black = (rand() % 10) - 5;
				}
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
					for(
						int DeltaIndex = 0;
						DeltaIndex < Player_Count;
						DeltaIndex++
					)
					{
						player_resources* ResourceDelta = (
							&DeckCard->PlayDelta[DeltaIndex]
						);
						ResourceDelta->Red = -1;
						ResourceDelta->Green = 1;
						ResourceDelta->Blue = (rand() % 10) - 5;
						ResourceDelta->White = (rand() % 10) - 5;
						ResourceDelta->Black = (rand() % 10) - 5;

						ResourceDelta = &DeckCard->TapDelta[DeltaIndex];
						ResourceDelta->Red = 1;
						ResourceDelta->Green = -1;
						ResourceDelta->Blue = (rand() % 10) - 5;
						ResourceDelta->White = (rand() % 10) - 5;
						ResourceDelta->Black = (rand() % 10) - 5;
					}

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
					DeckCard->Next = NULL;
					DeckCard->Previous = PrevDeckCard;
					PrevDeckCard->Next = DeckCard;
					Deck->OutOfDeckLength++;
				}
				DeckCard->Next = Deck->OutOfDeck;
			}
			// SECTION STOP: Shuffle deck
		}

		GameState->PlayerResources = PushArray(
			&GameState->TransientArena, Player_Count, player_resources
		);
		GameState->Hands = PushArray(
			&GameState->TransientArena, Player_Count, card_set
		);
		GameState->Tableaus = PushArray(
			&GameState->TransientArena, Player_Count, card_set
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
		CardSet->Type = CardSet_Hand;
		CardSet->CardCount = 0;
		CardSet->ScreenWidth = ScreenWidthInWorld;
		CardSet->YPos = 0.0f;
		CardSet->CardWidth = CardWidth;

		CardSet = &GameState->Hands[Player_Two];
		CardSet->Type = CardSet_Hand;
		CardSet->CardCount = 0;
		CardSet->ScreenWidth = ScreenWidthInWorld;
		CardSet->YPos = ScreenDimInWorld.Y - CardHeight;
		CardSet->CardWidth = CardWidth;

		CardSet = &GameState->Tableaus[Player_One];
		CardSet->Type = CardSet_Tableau;
		CardSet->CardCount = 0;
		CardSet->ScreenWidth = ScreenWidthInWorld;
		CardSet->YPos = CardHeight + HandTableauMargin;
		CardSet->CardWidth = CardWidth;

		CardSet = &GameState->Tableaus[Player_Two];
		CardSet->Type = CardSet_Tableau;
		CardSet->CardCount = 0;
		CardSet->ScreenWidth = ScreenWidthInWorld;
		CardSet->YPos = (
			ScreenDimInWorld.Y - (2 * CardHeight) - HandTableauMargin 
		);
		CardSet->CardWidth = CardWidth;
		
		DrawFullHand(GameState, Player_One, CardWidth, CardHeight);
		DrawFullHand(GameState, Player_Two, CardWidth, CardHeight);

		GameState->TurnTimer = 20.0f;

		for(int PlayerIndex = 0; PlayerIndex < Player_Count; PlayerIndex++)
		{
			player_resources* PlayerResources = (
				&GameState->PlayerResources[PlayerIndex]
			);
			// TODO: initialize to 0?
			PlayerResources->Red = rand() % 10;
			PlayerResources->Green = rand() % 10; 
			PlayerResources->Blue = rand() % 10; 
			PlayerResources->White = rand() % 10; 
			PlayerResources->Black = rand() % 10;  
		}

		Memory->IsInitialized = true;

		// TODO: remove me!
		// PlaySound(
		// 	&GameState->PlayingSoundList,
		// 	WavHandle_TestMusic,
		// 	&GameState->TransientArena
		// );

		// TODO: remove me!
		{
			vector2 ScreenCenter = Vector2(
				BackBuffer->Width / 2.0f, BackBuffer->Height / 2.0f
			);
			GameState->TestParticleSystem = MakeParticleSystem(
				&GameState->TransientArena,
				BitmapHandle_TestBitmap,
				1.0f,
				Vector2(35.0f, 35.0f),
				Vector2(45.0f, 45.0f),
				ScreenCenter - Vector2(5.0f, 5.0f),
				ScreenCenter + Vector2(5.0f, 5.0f),
				PI32,
				2 * PI32,
				1.0f,
				5.0f,
				Vector4(1.0f, 1.0f, 1.0f, 1.0f),
				Vector4(1.0f, 1.0f, 1.0f, 1.0f),
				Vector4(0.0f, 0.0f, 0.0f, -0.01f),
				Vector4(0.0f, 0.0f, 0.0f, -0.05f),
				0.25f,
				256
			);
		}
	}

	GameState->Time += DtForFrame;

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
				PlaySound(
					&GameState->PlayingSoundList,
					WavHandle_Bloop00,
					&GameState->TransientArena
				);
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
						if(Card->SetType == CardSet_Hand)
						{
							RemoveCardAndAlign(
								&GameState->Hands[Card->Owner], Card
							);
							AddCardAndAlign(
								&GameState->Tableaus[Card->Owner], Card
							);

							for(
								int PlayerIndex = 0; 
								PlayerIndex < Player_Count;
								PlayerIndex++
							)
							{
								ChangeResources(
									&GameState->PlayerResources[PlayerIndex],
									&Card->PlayDelta[PlayerIndex]
								);
							}
						}
						else if(Card->SetType == CardSet_Tableau)
						{
							for(
								int PlayerIndex = 0; 
								PlayerIndex < Player_Count;
								PlayerIndex++
							)
							{
								ChangeResources(
									&GameState->PlayerResources[PlayerIndex],
									&Card->TapDelta[PlayerIndex]
								);
							}
						}
						else
						{
							ASSERT(false);
						}
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
					switch(KeyboardEvent->Code)
					{
						case(0x44): // NOTE: D V-code
						{
							// TODO: consider pulling this predicate out to an 
							// CONT: inline function called "KeyUp" or something
							if(!KeyboardEvent->IsDown && KeyboardEvent->WasDown)
							{
								GameState->OverlayDebugInfo = (
									!GameState->OverlayDebugInfo
								);
							}
							break;
						}
					}
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
	// SECTION START: Turn timer update
	{
		GameState->TurnTimer -= DtForFrame;
		if(GameState->TurnTimer <= 0)
		{
			GameState->TurnTimer = 20.0f;
			GameState->CurrentTurn = (
				(GameState->CurrentTurn	== Player_Two) ? Player_One : Player_Two
			);
		}
	}
	// SECTION STOP: Turn timer update
	// SECTION START: Card update
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
			}
			Card++;
		}
	}
	// SECTION STOP: Card update

	// SECTION START: Push render entries
	PushClear(&GameState->RenderGroup, Vector4(0.25f, 0.25f, 0.25f, 1.0f));
	// SECTION START: Push turn timer
	{
		int32_t TurnTimerCeil = Int32Ceil(GameState->TurnTimer);
		uint32_t MaxTurnTimerCharacters = 10;
		char* TurnTimerString = PushArray(
			&GameState->FrameArena, MaxTurnTimerCharacters, char
		);
		char* PlayerIndicator = NULL;
		if(GameState->CurrentTurn == Player_One)
		{
			PlayerIndicator = "P1";
		}
		else if(GameState->CurrentTurn == Player_Two)
		{
			PlayerIndicator = "P2";
		}
		ASSERT(PlayerIndicator != NULL);
		snprintf(
			TurnTimerString,
			MaxTurnTimerCharacters,
			"%s: %d",
			PlayerIndicator,
			TurnTimerCeil
		);
		PushText(
			&GameState->RenderGroup,
			&GameState->Assets,
			FontHandle_TestFont,
			TurnTimerString,
			MaxTurnTimerCharacters,
			50.0f,
			Vector2(
				BackBuffer->Width - 150.0f, 
				(BackBuffer->Height / 2.0f)
			),
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			&GameState->FrameArena
		);
	}
	// SECTION STOP: Push turn timer
	// SECTION START: Push cards
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
				PushSizedBitmap(
					&GameState->RenderGroup,
					&GameState->Assets,
					BitmapHandle_TestCard2,
					GetCenter(Card->Rectangle),
					Vector2(Card->Rectangle.Dim.X, 0.0f),
					Vector2(0.0f, Card->Rectangle.Dim.Y),
					Card->Color
				);
			}
			Card++;
		}
	}
	// SECTION STOP: Push cards
	// SECTION START: Push resources
	{
		uint32_t MaxResourceStringSize = 40;
		char* ResourceString = PushArray(
			&GameState->FrameArena, MaxResourceStringSize, char
		);
		float Padding = 15.0f;

		player_resources* PlayerResources = (
			&GameState->PlayerResources[Player_One]
		);
		snprintf(
			ResourceString,
			MaxResourceStringSize,
			"R:%d\nG:%d\nB:%d\nW:%d\nB:%d",
			PlayerResources->Red,
			PlayerResources->Green,
			PlayerResources->Blue,
			PlayerResources->White,
			PlayerResources->Black
		);
			
		PushText(
			&GameState->RenderGroup,
			&GameState->Assets,
			FontHandle_TestFont,
			ResourceString,
			MaxResourceStringSize,
			15.0f,
			Vector2(
				BackBuffer->Width - 50.0f, 
				BackBuffer->Height / 2.0f - Padding
			),
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			&GameState->FrameArena
		);

		PlayerResources = &GameState->PlayerResources[Player_Two];
		snprintf(
			ResourceString,
			MaxResourceStringSize,
			"R:%d\nG:%d\nB:%d\nW:%d\nB:%d",
			PlayerResources->Red,
			PlayerResources->Green,
			PlayerResources->Blue,
			PlayerResources->White,
			PlayerResources->Black
		);	
		PushText(
			&GameState->RenderGroup,
			&GameState->Assets,
			FontHandle_TestFont,
			ResourceString,
			MaxResourceStringSize,
			15.0f,
			Vector2(
				BackBuffer->Width - 50.0f, 
				(BackBuffer->Height / 2.0f) + 80.0f + Padding
			),
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			&GameState->FrameArena
		);
	}
	// SECTION STOP: Push resources
	// SECTION STOP: Player render entries

#if 0 // NOTE: tests for bitmaps
	PushSizedBitmap(
		&GameState->RenderGroup,
		&GameState->Assets,
		BitmapHandle_TestBackground,
		Vector2((BackBuffer->Width / 2.0f), (BackBuffer->Height / 2.0f)),
		Vector2(BackBuffer->Width, 0),
		Vector2(0, BackBuffer->Height),
		Vector4(1.0f, 1.0f, 1.0f, 1.0f)		
	);

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
		&GameState->Assets,
		BitmapHandle_TestBitmap,
		Center,
		XAxis,
		YAxis,
		Vector4(1.0f, 1.0f, 1.0f, 1.0f)
	);

	PushText(
		&GameState->RenderGroup,
		&GameState->Assets,
		FontHandle_TestFont,
		"A hello world\nA test is here\nAnother line\nA\nA\nA\nA\nA",
		256,
		50.0f,
		Center + Vector2(0.0f, (BackBuffer->Height / 4.0f)),
		Vector4(1.0f, 1.0f, 1.0f, 1.0f),
		&GameState->FrameArena
	);

	basis RectBasis = MakeBasis(Vector2(0, 0), Vector2(1, 0), Vector2(0, 1));
	PushRect(
		&GameState->RenderGroup,
		&RectBasis,
		MakeRectangle(Vector2(0, 0), Vector2(25, 25)),
		Vector4(1.0f, 1.0f, 1.0f, 1.0f)
	);
#endif

#if 0 // NOTE: tests for particle systems
	UpdateParticleSystem(&GameState->TestParticleSystem);
	PushParticles(
		&GameState->RenderGroup,
		&GameState->Assets,
		&GameState->TestParticleSystem
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
}

void GameFillSound(game_memory* Memory, game_sound_output_buffer* SoundBuffer)
{
	ASSERT(sizeof(game_state) <= Memory->PermanentStorageSize);
	game_state* GameState = (game_state*) Memory->PermanentStorage;

	MixSounds(
		SoundBuffer,
		&GameState->FrameArena,
		&GameState->Assets,
		&GameState->PlayingSoundList
	);
}

void HandleGameDebug(game_memory* Memory, game_offscreen_buffer* BackBuffer)
{
	ASSERT(sizeof(game_state) <= Memory->PermanentStorageSize);
	game_state* GameState = (game_state*) Memory->PermanentStorage;
	if(GameState->OverlayDebugInfo)
	{
		loaded_bitmap DrawBuffer = {};
		DrawBuffer.Width = BackBuffer->Width;
		DrawBuffer.Height = BackBuffer->Height;
		DrawBuffer.Pitch = BackBuffer->Pitch;
		DrawBuffer.Memory = BackBuffer->Memory;

		uint32_t MaxDebugInfoStringSize = 1024; 
		char* DebugInfoString = PushArray(
			&GameState->FrameArena, MaxDebugInfoStringSize, char
		);
		char* CopyTo = DebugInfoString;
		uint32_t CharactersRemaining = MaxDebugInfoStringSize;
		for(
			uint32_t DebugRecordIndex = 0; 
			DebugRecordIndex < MAX_DEBUG_RECORDS;
			DebugRecordIndex++
		)
		{
			debug_record* DebugRecord = &GlobalDebugRecords[DebugRecordIndex];
			if(DebugRecord->HitCount)
			{
				// TODO: replace with os non-specific path delimiters
				char* LastDelimiter = FindLast(
					DebugRecord->FileName, 256, '\\'
				);
				if(LastDelimiter == NULL)
				{
					continue;
				}

				char* FileName = LastDelimiter + 1;
				int WrittenBytes = snprintf(
					CopyTo,
					CharactersRemaining,
					"%s:%d %I64ucy %uh %I64ucy/h\n",
					DebugRecord->FunctionName,
					DebugRecord->LineNumber,
					DebugRecord->CycleCount,
					DebugRecord->HitCount,
					DebugRecord->CycleCount / DebugRecord->HitCount
				);
				CopyTo += WrittenBytes;
				CharactersRemaining -= WrittenBytes;
				DebugRecord->CycleCount = 0;
				DebugRecord->HitCount = 0;
			}
		}

		PushText(
			&GameState->RenderGroup,
			&GameState->Assets,
			FontHandle_TestFont,
			DebugInfoString,
			MaxDebugInfoStringSize,
			20.0f,
			Vector2(
				0.0f, 
				BackBuffer->Height - 30.0f
			),
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			&GameState->FrameArena
		);

		RenderGroupToOutput(&GameState->RenderGroup, &DrawBuffer);	
		ResetMemArena(&GameState->FrameArena);
	}
}