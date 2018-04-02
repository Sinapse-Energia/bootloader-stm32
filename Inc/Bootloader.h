#ifndef __STM32_BOOTLOADER_H
#define __STM32_BOOTLOADER_H

// --- INCLUDES ---
#include <Crc32.h>
#include <Socket_bank.h>
#include "stm32f4xx_hal.h"
#include "Flash_NVM.h"
#include "Definitions.h"
#include "Flash_NVM.h"


// --- DEFINES ---
// Read write memory buffer (quicker operation)
#define BOOT_BUFFER_SIZE                    (4*1024lu)//128
// MCU NVM Flash Memory addresses
#define BOOT_APPLICATION_ADDR				FlashNVM_GetBankStartAddress(FLASH_BANK_APPLICATION)

// Firmware available answers
typedef enum
{
	NO_NEW_FIRMWARE = 0,
	NEW_AVAILABLE_FIRMWARE
} FIRMWARE_ERRORS;

// Bootloader errors
typedef enum
{
	BOOT_OK = 0,
	BOOT_ERR_VERIFY,
	BOOT_ERR_ADDRESS,
    BOOT_ERR_CRC,
	BOOT_ERR_CONNECTION,
	BOOT_ERR_NODATA,
    BOOT_ERR_UNKNOWN
} BOOT_ERRORS;

// WiFi/Eth errors
typedef enum
{
	ETHWIFI_FTP_FOLDER_FOUND = 0,
	ERROR_ETHWIFI_NOT_FOUND,
	ERROR_ETHWIFI_DISABLED,
	ERROR_ETHWIFI_FTP_NOT_CONNECT,
	ERROR_ETHWIFI_FOLDER_NOT_FOUND
} ETHWIFI_ERRORS;

// GPRS modem errors
typedef enum GPRS_ERRORS
{
	GPRS_FTP_FOLDER_FOUND = 0,
	ERROR_GPRS_NOT_FOUND,
	ERROR_GPRS_DISABLED,
	ERROR_GPRS_FTP_NOT_CONNECT,
	ERROR_GPRS_FOLDER_NOT_FOUND
} GPRS_ERRORS;
		
typedef enum {
	RGB_COLOR_OFF = 0,
	RGB_COLOR_WHITE,
	RGB_COLOR_YELLOW,
	RGB_COLOR_GREEN,
	RGB_COLOR_RED,
	RGB_COLOR_CYAN
} RGB_Color_type;

// Public functions
void Boot_RebootMCU(void);
uint8_t Boot_StartApplication(void);
uint8_t Boot_PerformFirmwareUpdate(void);

void RGB_Color_Set(RGB_Color_type color);
void RGB_Color_Blink(RGB_Color_type color);

#endif // __STM32_BOOTLOADER_H
