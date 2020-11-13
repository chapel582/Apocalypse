#ifndef APOCALYPSE_HOST_GAME_H

#include "apocalypse_socket.h"
#include "apocalypse_platform.h"

struct host_game_state
{
	bool Listening;
	vector2 ScreenDimInWorld;
	platform_socket* ListenSocket;
	platform_socket* ClientSocket;
};

void StartHostGamePrep(game_state* GameState);
void StartHostGame(
	game_state* GameState, uint32_t WindowWidth, uint32_t WindowHeight
);
void UpdateAndRenderHostGame(
	game_state* GameState,
	host_game_state* SceneState,
	uint32_t WindowWidth,
	uint32_t WindowHeight,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
);

#define APOCALYPSE_HOST_GAME_H
#endif