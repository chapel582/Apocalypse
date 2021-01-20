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
	game_state* GameState,
	platform_socket* Socket,
	packet_header* Header,
	memory_arena* Arena
)
{
	// NOTE: this function assumes that the whole packet begins at the header in
	// CONT: memory and is contiguous
	// socket_send_data_args* SendStatePacketArgs = PushStruct(
	// 	Arena, socket_send_data_args
	// );
	// SendStatePacketArgs->Socket = Socket;
	// SendStatePacketArgs->Buffer = Header;
	// SendStatePacketArgs->BufferSize = Header->DataSize;
	// SendStatePacketArgs->DataSize = Header->DataSize;
	// PlatformAddJob(
	// 	GameState->JobQueue,
	// 	SocketSendDataJob,
	// 	SendStatePacketArgs,
	// 	JobPriority_SendPacket
	// );
	PlatformSocketSend(Socket, Header, Header->DataSize);
}