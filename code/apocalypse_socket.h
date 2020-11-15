#ifndef APOCALYPSE_SOCKET_H

#include "apocalypse_platform.h"

// TODO: can we make this terse?
typedef enum
{
	Packet_Ready,
	Packet_StateUpdate
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
	uint64_t FrameId;
	uint32_t DataSize;
};
#pragma pack(pop)

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