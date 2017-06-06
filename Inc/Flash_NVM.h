#ifndef __FLASH__NVM_H
#define __FLASH__NVM_H

#include "stm32f4xx_hal.h"


// Divide Flash on three banks:
// Bank B (Bootloader)
// Bank A (Application)
// Bank C (Copy of new application firmware in update procedure)
#define FLASH_BANK_BOOTLOADER			0
#define FLASH_BANK_APPLICATION			1
#define FLASH_BANK_COPY					2

// Define Bank Sectors
// Bootloader
#define FLASH_BANKB_START_SECTOR		FLASH_SECTOR_0
#define FLASH_BANKB_SECTORS				2
// Application
#define FLASH_BANKA_START_SECTOR		FLASH_SECTOR_2
#define FLASH_BANKA_SECTORS				5
// Application copy
#define FLASH_BANKC_START_SECTOR		FLASH_SECTOR_7
#define FLASH_BANKC_SECTORS				5


// Public Functions
HAL_StatusTypeDef FlashNVM_EraseBank(uint8_t fl_bank);
HAL_StatusTypeDef FlashNVM_Read(uint32_t start_address, uint8_t* data_out, uint32_t size);
HAL_StatusTypeDef FlashNVM_Write(uint32_t start_address, const uint8_t* data_in, uint32_t size);
uint32_t FlashNVM_GetBankSize(uint8_t fl_bank);
uint32_t FlashNVM_GetBankStartAddress(uint8_t fl_bank);

#endif // __FLASH_H
