/*
 * Shared.c
 *
 * Shared memory access library
 *
 */

#include "Shared.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "Definitions.h"
#include "Flash_NVM.h"
#include "crc16.h"

static void InitializeData(SharedMemoryData* data)
{
	memset(data, 0, sizeof(SharedMemoryData));
	data->variables.UPDFW = UPDFW_UNSET;
	strncpy(data->variables.FW_NAME, HTTP_SERVER_FW_FILENAME, 63);
	strncpy(data->variables.FW_SERVER_URI, HTTP_SERVER_IP, 63);
	data->variables.PORT = HTTP_SERVER_PORT;
	strncpy(data->variables.PROTOCOL, "HTTP", 7);
}

bool ReadSharedMemory(SharedMemoryData* outData)
{
	// Check pointer
	if (outData == NULL) return false;

	// Initialize outData with default values
	InitializeData(outData);

	// Read from flash
	uint32_t fl_addr = FlashNVM_GetBankStartAddress(FLASH_BANK_SHARED);
	if (FlashNVM_Read(fl_addr, (uint8_t*)(outData), sizeof(SharedMemoryData))
			== HAL_ERROR)
	{
		// Flash read error - initialize outData with default values
		InitializeData(outData);
		WriteSharedMemory(outData);
		return false;
	}

	// Check CRC16
	uint16_t crc16 = calcCRC16((uint8_t*)(&(outData->variables)),
			sizeof(SharedMemoryVariables));
	if (crc16 != outData->crc16)
	{
		// Bad CRC16 - initialize outData with default values
		InitializeData(outData);
		WriteSharedMemory(outData);
		return false;
	}

	// If read and CRC16 is on - return true
	return true;
}

bool WriteSharedMemory(SharedMemoryData* inData)
{
	// Check pointer
	if (inData == NULL) return false;

	// Calculate CRC16
	uint16_t crc16 = calcCRC16((uint8_t*)(&(inData->variables)),
			sizeof(SharedMemoryVariables));
	inData->crc16 = crc16;

	// Erase flash
	if (FlashNVM_EraseBank(FLASH_BANK_SHARED) == HAL_ERROR) return false;

	// Write flash
	uint32_t fl_addr = FlashNVM_GetBankStartAddress(FLASH_BANK_SHARED);
	if (FlashNVM_Write(fl_addr, (uint8_t*)(inData), sizeof(SharedMemoryData))
			== HAL_ERROR) return false;

	return true;
}