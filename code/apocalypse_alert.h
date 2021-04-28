#ifndef APOCALYPSE_ALERT_H

#include "apocalypse_platform.h"
#include "apocalypse.h"

struct alert
{
	float DisplayMessageUntil;
	char MessageBuffer[256]; // TODO: consider making this non-static
};

inline alert MakeAlert()
{
	alert Result = {};
	return Result;
}

void DisplayMessageFor(
	game_state* GameState, alert* Alert, char* Message, float Time
);
void PushAlert(
	alert* Alert,
	game_state* GameState,
	vector2 Center,
	float TextSize
);
void PushCenteredAlert(
	alert* Alert,
	game_state* GameState,
	vector2 WindowDim
);

#define APOCALYPSE_ALERT_H
#endif
