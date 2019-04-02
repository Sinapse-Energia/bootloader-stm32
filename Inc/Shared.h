/*
 * Shared.h
 *
 * Shared memory access library
 *
 */

#ifndef SHARED_H_
#define SHARED_H_

#include <stdint.h>
#include <stdbool.h>
#include "Definitions.h"

#define UPDFW_UNSET 			-1

typedef enum {
	BootloaderProtocol_HTTP = 0,
	BootloaderProtocol_HTTPS,
	BootloaderProtocol_FTP,
	BootloaderProtocol_FTPS,
} BootloaderProtocol;

/**
 * Shared memory data structure
 */

typedef struct {
	uint8_t UPDFW_COUNT;
	int8_t UPDFW;
	char FW_SERVER_URI[64];
	uint16_t PORT;
	char PROTOCOL[8];
	char PATH[64];
	char FW_NAME[64];
	char USER[64];
	char PASSWORD[64];
	char FW_VERSION[8];
	uint8_t RELAYS[8];
    int8_t RELAY_PER_STATE[8];
    int32_t RELAY_PER_NTIMES[8];
    uint8_t RELAY_PER_H1[8];
    uint8_t RELAY_PER_M1[8];
    uint8_t RELAY_PER_H2[8];
    uint8_t RELAY_PER_M2[8];
} SharedMemoryVariables;

typedef struct {
	SharedMemoryVariables variables;
	uint16_t crc16;
} SharedMemoryData;

bool ReadSharedMemory(SharedMemoryData* outData);
bool WriteSharedMemory(SharedMemoryData* inData);

#endif /* SHARED_H_ */
