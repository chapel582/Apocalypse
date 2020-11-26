#ifndef APOCALYPSE_SOCKET_H

#include "apocalypse.h"
#include "apocalypse_platform.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_player_id.h"
#include "apocalypse_card_game.h"

// TODO: can we make this terse?
typedef enum
{
	Packet_Ready,
	Packet_StateUpdate,
	Packet_CardUpdate,
	Packet_DeckData,
	Packet_RandSeed,
	Packet_PlayCard,
	
	// NOTE: once TakeControl packet is received, follower will stop ignoring 
	// CONT: leader's updates for this entity
	Packet_TakeControl
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

struct take_control_payload
{
	uint32_t EntityId;
};
struct take_control_packet
{
	packet_header Header;
	take_control_payload Payload;
};

struct card_update_payload
{
	uint32_t CardId;
	uint32_t DefId;
	player_id Owner;
	card_set_type SetType;
};
struct card_update_packet
{
	packet_header Header;
	card_update_payload Payload;
};

struct state_update_payload
{
	player_id CurrentTurn;
	float TurnTimer;
	float NextTurnTimer;
	uint32_t NextId;
};
struct state_update_packet
{
	packet_header Header;
	state_update_payload Payload;
};

struct play_card_payload
{
	uint32_t CardId;
	uint32_t EntityId;
};
struct play_card_packet
{
	packet_header Header;
	play_card_payload Payload;
};

struct deck_data_payload
{
	loaded_deck Deck;
};
struct deck_data_packet
{
	packet_header Header;
	deck_data_payload Payload;
};
#pragma pack(pop)

void InitPacketHeader(
	game_state* GameState, packet_header* Header, packet_type Type
)
{
	*Header = {};
	Header->PacketId = GameState->PacketIdTracker++;
	Header->Type = Type;
	switch(Type)
	{
		case(Packet_StateUpdate):
		{
			Header->DataSize = sizeof(state_update_packet);
			break;
		}
		case(Packet_CardUpdate):
		{
			Header->DataSize = sizeof(card_update_packet);
			break;
		}
		case(Packet_DeckData):
		{
			Header->DataSize = sizeof(deck_data_packet);
			break;
		}
		case(Packet_PlayCard):
		{
			Header->DataSize = sizeof(play_card_packet);
			break;
		}
		case(Packet_TakeControl):
		{
			Header->DataSize = sizeof(take_control_packet);
			break;
		}
		default:
		{
			ASSERT(false);
			break;
		}
	}
	Header->FrameId = GameState->FrameCount;
}

struct socket_send_data_args
{
	platform_socket* Socket;
	void* Buffer;
	uint32_t BufferSize;
	uint32_t DataSize;
};
void SocketSendDataJob(void* Data);
void SocketSendData(
	game_state* GameState,
	platform_socket* Socket,
	packet_header* Header,
	memory_arena* Arena
);

#define APOCALYPSE_SOCKET_H
#endif