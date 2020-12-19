#ifndef APOCALYPSE_JOIN_GAME_H

#include "apocalypse.h"
#include "apocalypse_platform.h"

typedef enum
{
	ConnectionState_IpEntry,
	ConnectionState_InProgress,
	ConnectionState_Complete,	
	ConnectionState_Error
} connection_state;

struct connection_args
{
	platform_socket* ConnectSocket;
	char IpAddress[32];
	connection_state ConnectionState;
};

struct join_game_state
{
	ui_context UiContext;
	text_input IpInput;
	rectangle IpInputRectangle;

	vector2 ScreenDimInWorld;
	platform_socket* ConnectSocket;
	
	void* PacketBuffer;
	uint32_t PacketBufferSize;

	connection_args ConnectionArgs;
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