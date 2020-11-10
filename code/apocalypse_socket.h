#ifndef APOCALYPSE_SOCKET_H

#include "apocalypse_platform.h"

struct socket_send_data_args
{
	platform_socket* Socket;
	void* Buffer;
	uint32_t BufferSize;
	uint32_t DataSize;
};
void SocketSendDataJob(void* Data);

#define APOCALYPSE_SOCKET_H
#endif