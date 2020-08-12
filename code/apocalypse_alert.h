#ifndef APOCALYPSE_ALERT_H

#include "apocalypse_platform.h"
#include "apocalypse.h"

struct alert
{
	float DisplayMessageUntil;
	char MessageBuffer[256]; // TODO: consider making this non-static
};

inline alert Alert()
{
	alert Result = {};
	return Result;
}

void DisplayMessageFor(
	game_state* GameState, alert* Alert, char* Message, float Time
);
void PushCenteredAlert(
	alert* Alert, game_state* GameState, game_offscreen_buffer* BackBuffer
);

#define APOCALYPSE_ALERT_H
#endif
