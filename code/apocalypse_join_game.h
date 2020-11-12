#ifndef APOCALYPSE_JOIN_GAME_H

#include "apocalypse.h"
#include "apocalypse_platform.h"

typedef enum
{
	ConnectToServerResult_InProgress,
	ConnectToServerResult_Complete,	
	ConnectToServerResult_Error
} connect_to_server_result;

struct connect_to_server_args
{
	platform_socket* ConnectSocket;
	connect_to_server_result ConnectionResult;
};

struct join_game_state
{
	vector2 ScreenDimInWorld;
	platform_socket* ConnectSocket;
	
	void* PacketBuffer;
	uint32_t PacketBufferSize;

	connect_to_server_args* ConnectToServerArgs;
};

void StartJoinGamePrep(game_state* GameState);
void StartJoinGame(
	game_state* GameState, uint32_t WindowWidth, uint32_t WindowHeight
);
void UpdateAndRenderJoinGame(
	game_state* GameState,
	join_game_state* SceneState,
	uint32_t WindowWidth,
	uint32_t WindowHeight,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
);

#define APOCALYPSE_JOIN_GAME_H
#endif