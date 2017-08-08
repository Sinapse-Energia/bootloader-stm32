#ifndef __FLASH__NVM_H
#define __FLASH__NVM_H

#include "stm32f0xx_hal.h"
#include "Definitions.h"


// Divide Flash on three banks:
// Bank B (Bootloader)
// Bank A (Application)
// Bank C (Copy of new application firmware in update procedure)
typedef enum
{
	FLASH_BANK_BOOTLOADER = 0,
	FLASH_BANK_APPLICATION,
	FLASH_BANK_COPY
} FLASH_BANK;


// Public Functions
HAL_StatusTypeDef FlashNVM_EraseBank(FLASH_BANK fl_bank);
HAL_StatusTypeDef FlashNVM_Read(uint32_t start_address, uint8_t* data_out, uint32_t size);
HAL_StatusTypeDef FlashNVM_Write(uint32_t start_address, const uint8_t* data_in, uint32_t size);
uint32_t FlashNVM_GetBankSize(FLASH_BANK fl_bank);
uint32_t FlashNVM_GetBankStartAddress(FLASH_BANK fl_bank);

#endif // __FLASH_H
