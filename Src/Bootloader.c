#include "Bootloader.h"

// R/W buffer
char boot_buff[BOOT_BUFFER_SIZE];

/**
  * @brief  Close bootloader and start Application
  * @param  none
  * @retval true if OK,  otherwise return false
  */
uint8_t Boot_StartApplication(void)
{
    typedef void (*pFunction)(void);
    pFunction appEntry;
    uint32_t appStack;
    uint32_t app_check_address;
    uint32_t *app_check_address_ptr;

    app_check_address = BOOT_APPLICATION_ADDR;
    app_check_address_ptr = (uint32_t *) app_check_address;

    // Read the first location of application section
    // which contains the address of stack pointer
    // If it is 0xFFFFFFFF then the application section is empty
    if (*app_check_address_ptr == 0xFFFFFFFF) {
        return 0;
    }

    // Get the application stack pointer (First entry in the application vector table)
    appStack = (uint32_t) *((__IO uint32_t*)BOOT_APPLICATION_ADDR);

    // Get the application entry point (Second entry in the application vector table)
    appEntry = (pFunction) *(__IO uint32_t*) (BOOT_APPLICATION_ADDR + 4);

    // Reconfigure vector table offset register to match the application location
    SCB->VTOR = BOOT_APPLICATION_ADDR;

    // Set the application stack pointer
    __set_MSP(appStack);

    // Start the application
    appEntry();

    return 1; // OK (but in real app should never reach this point)
}



/**
  * @brief  Check for new application firmware available on remote FTP server
  * 		If No - Exit
  * 		If Yes- Download New firmware in BankC. Erase memory (BankA).
  * 		Flash (move) new firmware From BankC to BankA and verify it
  * @param  none
  * @retval Return BOOT_OK on success or BOOT_ERR_xxx on error
  */
BOOT_ERRORS Boot_PerformFirmwareUpdate(void)
{
    char* p;
    uint32_t len, i;
    uint32_t total_len, crc32;
    uint32_t fl_addr = FlashNVM_GetBankStartAddress(FLASH_BANK_COPY);
    uint32_t app_addr;
    unsigned int fw_len;

    // Clear buffer flash first
    FlashNVM_EraseBank(FLASH_BANK_COPY);

    Socket_Init(SOCKET_SRC_WIFI);

    // Check connection available
    // and select source
    // WiFi first
    sprintf(boot_buff, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", HTTP_SERVER_FW_FILENAME, HTTP_SERVER_IP);
    Socket_Write(SOCKET_SRC_WIFI, boot_buff, strlen(boot_buff));
    // Read answer
    Socket_ClearTimeout(SOCKET_SRC_WIFI);
    total_len = 0;
    while (!Socket_GetTimeout(SOCKET_SRC_WIFI)) {
        len = Socket_Read(SOCKET_SRC_WIFI, boot_buff, BOOT_BUFFER_SIZE);
        if (len) {
        	total_len += len;
        	FlashNVM_Write(fl_addr, (uint8_t*)boot_buff, len);
        	fl_addr += len;
        }
    }
    if (total_len < 10) {
    	printf("Server no answer!");
    	return BOOT_ERR_CONNECTION; //Err
    }

    // NVM flash operation

	// Find firmware length
    fl_addr = FlashNVM_GetBankStartAddress(FLASH_BANK_COPY);
	len = 0;
    for (i = 0; i < (len - 4); i++)
    {
    	FlashNVM_Read(fl_addr, (uint8_t*)boot_buff, 4);
    	if (strstr(boot_buff, "length")){
        	sscanf(boot_buff, "%ui", &fw_len);
        	fw_len -= 4; //dec CRC
        	break;
    	}
    }
    if (i == (len - 4)) {
    	printf("no data length found!");
    	return BOOT_ERR_NODATA; //Err
    }

	// Find firmware start position
    fl_addr = FlashNVM_GetBankStartAddress(FLASH_BANK_COPY);
	len = 0;
    for (i = 0; i < (len - 4); i++)
    {
    	FlashNVM_Read(fl_addr, (uint8_t*)boot_buff, 4);
    	p = strstr(boot_buff, "\r\n\r\n");
    	if (p) {
    		fl_addr += (4 + p - boot_buff);
    	}
    }
    if (i == (len - 4)) {
    	printf("no file found!");
    	return BOOT_ERR_NODATA; //Err
    }

    // Count firmware CRC (last 4 byte of the firmware will be - CRC32 checksum)
    crc32_Clear();
	len = 0;
    for (i = 0; i < fw_len; i++)
    {
    	FlashNVM_Read(fl_addr + i, (uint8_t*)boot_buff, 1);
    	crc32_Add(boot_buff[0]);
    }
    crc32 = crc32_Value();
    FlashNVM_Read(fl_addr + i, (uint8_t*)boot_buff, 4);
    // And check it
    if (!memcmp(boot_buff, &crc32, 4)) {
    	printf("Firmware CRC error!");
    	return BOOT_ERR_CRC; //Err
    }

    // Check is NEW firmware update available
    FlashNVM_Read(fl_addr + APP_VER_ADDR_HIGH, (uint8_t*)&len, 2);
    FlashNVM_Read(FlashNVM_GetBankStartAddress(FLASH_BANK_APPLICATION) + APP_VER_ADDR_HIGH, (uint8_t*)&i, 2);
    if (i >= len) {
    	printf("No new firmware available!");
    	return BOOT_OK;
    }

    // Update firmware (copy buffer to Application flash memory)
    FlashNVM_EraseBank(FLASH_BANK_APPLICATION);
    app_addr = FlashNVM_GetBankStartAddress(FLASH_BANK_APPLICATION);
    for (i = 0; i < fw_len; i += 128)
    {
    	FlashNVM_Read(fl_addr + i, (uint8_t*)boot_buff, BOOT_BUFFER_SIZE);
    	FlashNVM_Write(app_addr + i, (uint8_t*)boot_buff, BOOT_BUFFER_SIZE);
    }
    // Compare memories again?

    // Start new Application
    //Boot_StartApplication();

    return BOOT_OK;
}


/**
  * @brief  Reboot device via WDT or special command
  * @param  none
  * @retval none
  */
void Boot_RebootMCU(void)
{
    // Reset mcu
    NVIC_SystemReset();
}

