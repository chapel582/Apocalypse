#ifndef APOCALYPSE_LOST_CONNECTION_H

#include "apocalypse.h"
#include "apocalypse_platform.h"
#include "apocalypse_ui.h"
#include "apocalypse_dropdown.h"

#define RESOLUTION_CONFIG_PATH "./config/resolution.data"

struct lost_connection_args
{
	platform_socket ListenSocket;
	platform_socket ConnectSocket;
};

struct lost_connection_state
{
	ui_context UiContext;
	bool IsServer;
	bool Listening;
	platform_socket ListenSocket;
	platform_socket ConnectSocket;
};

void StartLostConnectionPrep(
	game_state* GameState,
	platform_socket* ConnectSocket,
	platform_socket* ListenSocket
);
void StartLostConnection(
	game_state* GameState,
	uint32_t WindowWidth,
	uint32_t WindowHeight,
	size_t ResetTo
);
void UpdateAndRenderLostConnection(
	game_state* GameState,
	lost_connection_state* SceneState,
	uint32_t WindowWidth,
	uint32_t WindowHeight,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
);

#define APOCALYPSE_LOST_CONNECTION_H
#endif APOCALYPSE_LOST_CONNECTION_H
