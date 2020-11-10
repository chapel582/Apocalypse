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