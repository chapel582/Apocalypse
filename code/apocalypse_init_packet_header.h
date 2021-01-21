#ifndef INIT_PACKET_HEADER

#include "apocalypse_socket.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_card_game.h"
#include "apocalypse_deck_selector.h"

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

#define INIT_PACKET_HEADER
#endif