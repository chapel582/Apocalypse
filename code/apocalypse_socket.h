#ifndef APOCALYPSE_SOCKET_H

#include "apocalypse.h"
#include "apocalypse_platform.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_packet_header.h"

#define MAX_SEND_LOG_SIZE MEGABYTES(1)

typedef enum
{
	ReadPacketResult_Unknown,
	ReadPacketResult_Complete,
	ReadPacketResult_Incomplete,
	ReadPacketResult_PeerReset,
	ReadPacketResult_Error
} read_packet_result;

struct packet_reader_data
{
	packet_header* Header;
	uint32_t HeaderBytesRead;
	void* Payload;
	uint32_t PayloadBytesRead;
	memory_arena* NetworkArena;
};

struct socket_send_data_args
{
	platform_socket* Socket;
	void* Buffer;
	uint32_t BufferSize;
	uint32_t DataSize;
};
platform_socket_send_result SocketSendData(
	game_state* GameState, platform_socket* Socket, packet_header* Header
);
void SocketSendErrorCheck(
	game_state* GameState,
	platform_socket* ConnectSocket,
	platform_socket* ListenSocket,
	packet_header* Header
);
platform_socket_send_result ThrottledSocketSendData(
	game_state* GameState, platform_socket* Socket, packet_header* Header
);
void ThrottledSocketSendErrorCheck(
	game_state* GameState,
	platform_socket* ConnectSocket,
	platform_socket* ListenSocket,
	packet_header* Header
);
read_packet_result ReadPacket(
	game_state* GameState,
	platform_socket* ConnectSocket,
	platform_socket* ListenSocket,
	packet_reader_data* PacketReader
);
void ReadPacketEnd(packet_reader_data* PacketReader);

#define APOCALYPSE_SOCKET_H
#endif