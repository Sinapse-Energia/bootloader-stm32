#ifndef __SOCKET_BANK_H
#define __SOCKET_BANK_H

#include <string.h>
#include "stm32f4xx_hal.h"
#include "Definitions.h"
#include "M95lite.h"


// Socket errors
typedef enum
{
	SOCKET_OK = 0,
	SOCKET_ERR_NO_CONNECTION,
	SOCKET_ERR_TIMEOUT,
	SOCKET_ERR_UNKNOWN
} SOCKET_STATUS;

// Possible socket sources
typedef enum
{
	SOCKET_SRC_NONE = 0,
	SOCKET_SRC_GPRS,
	SOCKET_SRC_WIFI
} SOCKETS_SOURCE;


// Public Functions
SOCKET_STATUS Socket_Init(SOCKETS_SOURCE s_in);
SOCKET_STATUS Socket_Connect(SOCKETS_SOURCE s_in);
SOCKET_STATUS Socket_Write(SOCKETS_SOURCE s_in, const char *data_in, int data_len);
int Socket_Read(SOCKETS_SOURCE s_in, char *buff_out, int buff_len);
void Socket_Clear(SOCKETS_SOURCE s_in);
void Socket_ClearTimeout(SOCKETS_SOURCE s_in);
uint8_t Socket_GetTimeout(SOCKETS_SOURCE s_in);

#endif // __SOCKET_BANK_H
