#include "apocalypse_alert.h"
#include "apocalypse_platform.h"
#include "apocalypse_vector.h"
#include "apocalypse.h"

void DisplayMessageFor(
	game_state* GameState, alert* Alert, char* Message, float Time
)
{
	Alert->DisplayMessageUntil = GameState->Time + Time;
	strcpy_s(
		Alert->MessageBuffer,
		ARRAY_COUNT(Alert->MessageBuffer),
		Message
	);
}

void PushCenteredAlert(
	alert* Alert,
	game_state* GameState,
	uint32_t WindowWidth,
	uint32_t WindowHeight
)
{
	if(GameState->Time < Alert->DisplayMessageUntil)
	{
		PushTextCentered(
			&GameState->RenderGroup,
			&GameState->Assets,
			FontHandle_TestFont,
			Alert->MessageBuffer,
			ARRAY_COUNT(Alert->MessageBuffer),
			50.0f,
			Vector2(WindowWidth / 2.0f, WindowHeight / 2.0f),
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			&GameState->FrameArena
		);
	}
}