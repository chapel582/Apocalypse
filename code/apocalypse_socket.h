#ifndef APOCALYPSE_SOCKET_H

#include "apocalypse.h"
#include "apocalypse_platform.h"
#include "apocalypse_memory_arena.h"
// #include "apocalypse_player_id.h"
// #include "apocalypse_card_game.h"
// #include "apocalypse_player_resources.h"

#define MAX_SEND_LOG_SIZE MEGABYTES(1)
typedef enum
{
	Packet_NotSet,
	Packet_LatencyCheck,
	Packet_LatencyCheckRsp,
	Packet_Ready,
	Packet_SwitchLeader,
	Packet_StateUpdate,
	Packet_CardUpdate,
	Packet_RemoveCard,
	Packet_DeckData,
	Packet_DeckUpdate,
	Packet_RandSeed
} packet_type;

/* TODO: make sure we handle all the following endianness cases
Little -> Little
Little -> Big
Big -> Big
Big -> Little
*/

#pragma pack(push, 1)
struct packet_header
{
	packet_type Type;
	uint64_t PacketId;
	uint64_t FrameId;
	uint32_t DataSize;
};

struct ready_packet
{
	packet_header Header;
};

struct rand_seed_payload
{
	uint32_t Seed;
};
struct rand_seed_packet
{
	packet_header Header;
	rand_seed_payload Payload;
};
#pragma pack(pop)

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
void ThrottledSocketSendData(
	game_state* GameState, platform_socket* Socket, packet_header* Header
);

#define APOCALYPSE_SOCKET_H
#endif