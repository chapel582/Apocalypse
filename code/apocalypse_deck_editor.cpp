#include "apocalypse_deck_editor.h"
#include "apocalypse_platform.h"
#include "apocalypse.h"

void StartDeckEditor(game_state* GameState)
{
	ResetMemArena(&GameState->TransientArena);
	GameState->SceneState = PushStruct(
		&GameState->TransientArena, deck_editor_state
	);
	ResetAssets(&GameState->Assets);
	deck_editor_state* SceneState = (deck_editor_state*) GameState->SceneState;

	SceneState->Definitions = DefineCards(&GameState->TransientArena);
	card_definitions* Definitions = SceneState->Definitions;
	vector2 Dim = Vector2(60.0f, 90.0f);
	uint32_t NumRows = 2;
	uint32_t NumCols = 4;
	float XMargin = 10.0f;
	float YMargin = 10.0f;
	for(uint32_t Row = 0; Row < NumRows; Row++)
	{
		for(uint32_t Col = 0; Col < NumCols; Col++)
		{
			uint32_t Index = NumCols * Row + Col;
			collection_card* CollectionCard = (
				SceneState->CollectionCards + Index
			);
			CollectionCard->Rectangle = MakeRectangle(
				Vector2(
					(Dim.X + XMargin)* Col,
					(Dim.Y + YMargin) * (NumRows - Row - 1)
				),
				Dim
			);
			if(Index < Definitions->NumCards)
			{
				CollectionCard->Active = true;
				CollectionCard->Definition = &Definitions->Array[Index];
			}
		}
	}
}

void StartDeckEditorCallback(void* Data)
{
	game_state* GameState = (game_state*) Data;
	GameState->Scene = SceneType_DeckEditor;
}

void UpdateAndRenderDeckEditor(
	game_state* GameState,
	deck_editor_state* SceneState,
	game_offscreen_buffer* BackBuffer,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
)
{
	PushClear(&GameState->RenderGroup, Vector4(0.25f, 0.25f, 0.25f, 1.0f));
	for(
		uint32_t Index = 0;
		Index < ARRAY_COUNT(SceneState->CollectionCards);
		Index++
	)
	{
		collection_card* Card = SceneState->CollectionCards + Index;
		if(Card->Active)
		{
			PushSizedBitmap(
				&GameState->RenderGroup,
				&GameState->Assets,
				BitmapHandle_TestCard2,
				GetCenter(Card->Rectangle),
				Vector2(Card->Rectangle.Dim.X, 0.0f),
				Vector2(0.0f, Card->Rectangle.Dim.Y),
				Vector4(1.0f, 1.0f, 1.0f, 1.0f)
			);
		}
	}
	// PushTextCentered(
	// 	&GameState->RenderGroup,
	// 	&GameState->Assets,
	// 	FontHandle_TestFont,
	// 	"Switched to deck editor!",
	// 	50,
	// 	50.0f,
	// 	Vector2(
	// 		BackBuffer->Width / 2.0f, 
	// 		BackBuffer->Height / 2.0f
	// 	),
	// 	Vector4(1.0f, 1.0f, 1.0f, 1.0f),
	// 	&GameState->FrameArena
	// );
}