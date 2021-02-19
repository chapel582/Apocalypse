#ifndef APOCALYPSE_JOIN_GAME_H

#include "apocalypse.h"
#include "apocalypse_platform.h"
#include "apocalypse_packet_header.h"
#include "apocalypse_text_input.h"
#include "apocalypse_ui.h"

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
	char IpAddress[64];
	connection_state ConnectionState;
};

typedef enum
{
	JoinGameType_Undefined,
	JoinGameType_Unknown,
	JoinGameType_NewGame,
	JoinGameType_ResumeGame
} join_game_type;

struct join_game_state
{
	ui_context UiContext;
	text_input IpInput;
	rectangle IpInputRectangle;

	vector2 ScreenDimInWorld;
	platform_socket* ConnectSocket;
	
	void* PacketBuffer;
	uint32_t PacketBufferSize;
	join_game_type JoinGameType;

	connection_args ConnectionArgs;

	packet_reader_data PacketReader;
};

#pragma pack(push, 1)
struct join_game_type_payload
{
	join_game_type Type;
};
struct join_game_type_packet
{
	packet_header Header;
	join_game_type_payload Payload;
};
#pragma pack(pop)

void StartJoinGamePrep(game_state* GameState, bool JoinNewGame);
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