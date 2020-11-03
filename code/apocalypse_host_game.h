#ifndef APOCALYPSE_HOST_GAME_H

struct host_game_state
{
	vector2 ScreenDimInWorld;
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