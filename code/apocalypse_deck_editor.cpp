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
	PushTextCentered(
		&GameState->RenderGroup,
		&GameState->Assets,
		FontHandle_TestFont,
		"Switched to deck editor!",
		50,
		50.0f,
		Vector2(
			BackBuffer->Width / 2.0f, 
			BackBuffer->Height / 2.0f
		),
		Vector4(1.0f, 1.0f, 1.0f, 1.0f),
		&GameState->FrameArena
	);
}