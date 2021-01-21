#include "apocalypse_socket.h"
#include "apocalypse.h"
#include "apocalypse_platform.h"

void SocketSendDataJob(void* Data)
{
	socket_send_data_args* Args = (socket_send_data_args*) Data;
	platform_socket* Socket = Args->Socket;
	void* Buffer = Args->Buffer;
	uint32_t DataSize = Args->DataSize;
	PlatformSocketSend(Socket, Buffer, DataSize);
}

void SocketSendData(
	game_state* GameState, platform_socket* Socket, packet_header* Header
)
{
	// NOTE: this function assumes that the whole packet begins at the header in
	// CONT: memory and is contiguous
	ASSERT(Header->Type != Packet_NotSet);
	PlatformSocketSend(Socket, Header, Header->DataSize);
}

void ThrottledSocketSendData(
	game_state* GameState, platform_socket* Socket, packet_header* Header
)
{
	// NOTE: this function assumes that the whole packet begins at the header in
	// CONT: memory and is contiguous
	ASSERT(Header->Type != Packet_NotSet);
	if(GameState->CanSendPackets)
	{
		PlatformSocketSend(Socket, Header, Header->DataSize);
	}
}

bool ReadPacket(
	platform_socket* ConnectSocket, packet_reader_data* PacketReader
)
{
	/*
	NOTE: 
	This function is for reading data into a packet with an option to fail out 
	part way through. 
	
	ConnectSocket is the socket used for reading.
	HeaderBytesRead is the number of bytes read from the header.
	PayloadBytesRead is the number of bytes read from the payload.
	NetworkArena is the arena used for . It is the caller's responsibility to 
	reset this arena
	*/ 
	memory_arena* NetworkArena = PacketReader->NetworkArena;
	uint32_t BytesRead = 0;
	if(PacketReader->Header == NULL)
	{
		// NOTE: need to read a packet
		PacketReader->Header = PushStruct(NetworkArena, packet_header);
		PacketReader->HeaderBytesRead = 0;
		PacketReader->PayloadBytesRead = 0;
	}
	while(PacketReader->HeaderBytesRead < sizeof(packet_header))
	{
		// TODO: error handling
		PlatformSocketRead(
			ConnectSocket,
			((uint8_t*) PacketReader->Header) + PacketReader->HeaderBytesRead,
			sizeof(packet_header) - PacketReader->HeaderBytesRead,
			&BytesRead
		);
		PacketReader->HeaderBytesRead += BytesRead;
		ASSERT(PacketReader->HeaderBytesRead <= sizeof(packet_header));

		if(PacketReader->HeaderBytesRead == sizeof(packet_header))
		{
			// NOTE: finished reading header
			break;
		}
		else
		{
			if(BytesRead == 0)
			{
				break;
			}
		}
	}
	packet_header* Header = PacketReader->Header;
	
	uint32_t PayloadSize = 0;
	if(
		Header == NULL ||
		PacketReader->HeaderBytesRead < sizeof(packet_header)
	)
	{
		// NOTE: could not read header, end
		return false;
	}
	else if(PacketReader->Payload == NULL)
	{
		// NOTE: need to read a new payload
		PayloadSize = Header->DataSize - sizeof(packet_header);
		PacketReader->Payload = PushSize(
			NetworkArena, Header->DataSize - sizeof(packet_header)
		);
	}

	if(PayloadSize == 0)
	{
		// NOTE: no payload to read. done
		return true;
	}

	while(PacketReader->PayloadBytesRead < PayloadSize)
	{
		platform_read_socket_result SocketReadResult = PlatformSocketRead(
			ConnectSocket,
			(
				((uint8_t*) PacketReader->Payload) +
				PacketReader->PayloadBytesRead
			),
			PayloadSize - PacketReader->PayloadBytesRead,
			&BytesRead
		);
		PacketReader->PayloadBytesRead += BytesRead;
		ASSERT(PacketReader->PayloadBytesRead <= PayloadSize);

		if(PacketReader->PayloadBytesRead >= PayloadSize)
		{
			// NOTE: finished reading payload
			break;
		}
		else
		{
			if(BytesRead == 0)
			{
				// NOTE: read no bytes, and we're not finished. 
				// CONT: end attempts for this call
				break;
			}
		}
	}

	return PacketReader->PayloadBytesRead >= PayloadSize;
}

void ReadPacketEnd(packet_reader_data* PacketReader)
{
	// NOTE: call this function at the end 
	ResetMemArena(PacketReader->NetworkArena);
	PacketReader->Header = NULL;
	PacketReader->Payload = NULL;
}