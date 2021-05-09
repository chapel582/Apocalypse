#ifndef APOCALYPSE_PACKET_HEADER_H

#include "apocalypse_memory_arena.h"

typedef enum
{
	Packet_NotSet,
	Packet_LatencyCheck,
	Packet_LatencyCheckRsp,
	Packet_Ready,
	Packet_SetLeader,
	Packet_SwitchLeader,
	Packet_StateUpdate,
	Packet_SetFrameCount,
	Packet_CardUpdate,
	Packet_RemoveCard,
	Packet_DeckData,
	Packet_CardDataSetUpdate,
	Packet_RandSeed,
	Packet_JoinGameType,
	Packet_CardStackUpdate,
	Packet_SyncDone
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
	uint64_t HeaderChecksum;
	uint64_t PayloadChecksum;
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

struct set_frame_count_payload
{
	uint64_t FrameCount;
};
struct set_frame_count_packet
{
	packet_header Header;
	set_frame_count_payload Payload;
};
#pragma pack(pop)

uint64_t GetHeaderChecksum(packet_header* Header)
{
	// NOTE: this needs to be called 
	return (
		Header->Type +
		Header->PacketId +
		Header->FrameId +
		Header->DataSize
	);
}

uint64_t GetPayloadChecksum(uint8_t* Payload, uint32_t PayloadSize)
{
	uint64_t Result = 0;
	for(uint32_t ByteIndex = 0; ByteIndex < PayloadSize; ByteIndex++)
	{
		Result += *(Payload + ByteIndex);
	}
	return Result;
}

void InitPacketHeader(
	game_state* GameState,
	packet_header* Header,
	packet_type Type,
	uint8_t* Payload
)
{
	// NOTE: Header->DataSize must be initialized by the caller
	// NOTE: Payload should be initialized before calling this function
	Header->PacketId = GameState->PacketIdTracker++;
	Header->Type = Type;
	
	Header->FrameId = GameState->FrameCount;
	Header->HeaderChecksum = GetHeaderChecksum(Header);
	if(Payload != NULL)
	{
		Header->PayloadChecksum = GetPayloadChecksum(
			Payload, Header->DataSize - sizeof(packet_header)
		);
	}
	else
	{
		Header->PayloadChecksum = 0;
	}
}

#define APOCALYPSE_PACKET_HEADER_H
#endif