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
	vector2 WindowDim
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
			0.5f * WindowDim,
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			&GameState->FrameArena
		);
	}
}