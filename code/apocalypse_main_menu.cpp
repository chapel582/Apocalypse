#include "apocalypse.h"
#include "apocalypse_assets.h"
#include "apocalypse_render_group.h"

void StartMainMenu(game_state* GameState)
{
	ResetAssets(&GameState->Assets);
	
	GameState->Scene = SceneType_MainMenu;
}

void UpdateAndRenderMainMenu(
	game_state* GameState,
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
		"You have switched scenes!",
		50,
		50.0f,
		Vector2(
			BackBuffer->Width / 2.0f, 
			(BackBuffer->Height / 2.0f)
		),
		Vector4(1.0f, 1.0f, 1.0f, 1.0f),
		&GameState->FrameArena
	);
}