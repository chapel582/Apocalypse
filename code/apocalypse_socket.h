#ifndef APOCALYPSE_SOCKET_H

#include "apocalypse.h"
#include "apocalypse_platform.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_player_id.h"
#include "apocalypse_card_game.h"
#include "apocalypse_player_resources.h"

#define MAX_SEND_LOG_SIZE MEGABYTES(1)
// TODO: can we make this terse?
typedef enum
{
	Packet_NotSet,
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

struct switch_leader_packet
{
	packet_header Header;
};

struct card_update_payload
{
	uint32_t CardId;
	uint32_t DefId;
	player_id Owner;
	card_set_type SetType;
	player_resources PlayDelta[Player_Count];
	player_resources TapDelta[Player_Count];
	
	player_resources TurnStartPlayDelta[Player_Count];
	player_resources TurnStartTapDelta[Player_Count];

	float TimeLeft;
	int32_t TapsAvailable;
	int32_t TimesTapped;
	int16_t Attack;
	int16_t TurnStartAttack;
	int16_t Health;
	int16_t TurnStartHealth;
	tableau_effect_tags TableauTags;
	stack_effect_tags StackTags;
};
struct card_update_packet
{
	packet_header Header;
	card_update_payload Payload;
};

struct remove_card_payload
{
	uint32_t CardId;
};
struct remove_card_packet
{
	packet_header Header;
	remove_card_payload Payload;
};

struct state_update_payload
{
	player_id CurrentTurn;
	float TurnTimer;
	float NextTurnTimer;
	player_id StackTurn;
	bool StackBuilding;
	player_resources PlayerResources[Player_Count];
	float PlayerLife[Player_Count];
};
struct state_update_packet
{
	packet_header Header;
	state_update_payload Payload;
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

struct deck_update_payload
{
	player_id Owner;
	uint32_t InDeckCount;
	uint32_t Offset;
};
struct deck_update_packet
{
	packet_header Header;
	deck_update_payload Payload;
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

void InitPacketHeader(
	game_state* GameState, packet_header* Header, packet_type Type
)
{
	*Header = {};
	Header->PacketId = GameState->PacketIdTracker++;
	Header->Type = Type;
	switch(Type)
	{
		case(Packet_Ready):
		{
			Header->DataSize = sizeof(ready_packet);
			break;
		}
		case(Packet_SwitchLeader):
		{
			Header->DataSize = sizeof(switch_leader_packet);
			break;
		}
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
		case(Packet_RemoveCard):
		{
			Header->DataSize = sizeof(remove_card_packet);
			break;
		}
		case(Packet_DeckData):
		{
			Header->DataSize = sizeof(deck_data_packet);
			break;
		}
		case(Packet_DeckUpdate):
		{
			// NOTE: data size cannot be determined by common code
			break;
		}
		case(Packet_RandSeed):
		{
			Header->DataSize = sizeof(rand_seed_packet);
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