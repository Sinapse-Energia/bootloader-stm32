#include "Bootloader.h"


extern uint8_t WDT_ENABLED;
extern IWDG_HandleTypeDef hiwdg;
extern UART_HandleTypeDef huart6;
// R/W buffer
char boot_buff[BOOT_BUFFER_SIZE];

void wlanRecvStop(UART_HandleTypeDef* huart);

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
    	if (LOG_WIFI==1) HAL_UART_Transmit(&huart6, (uint8_t*)"(BOOT NOT EXISTS APPLICATION FW!\r\n", 34,100); //Francis, for logging
        return 0;
    }

    if (LOG_WIFI==1) HAL_UART_Transmit(&huart6, (uint8_t*)"(BOOT Jumping to application program!\r\n", 36,100); //Francis, for logging

    __disable_irq();
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    __set_PRIMASK(1);

    HAL_RCC_DeInit();
    HAL_DeInit();

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
  * @brief  Check connection to server
  * @param  ssource - SOCKET_SRC_WIFI / SOCKET_SRC_GPRS
  * @retval Return true if connection OK
  */
uint8_t Boot_CheckConnection(SOCKETS_SOURCE ssource)
{
	char* p;
    uint32_t len, len_left;

    // Check connection available
    sprintf(boot_buff, "GET / HTTP/1.1\r\nHost: %s\r\n\r\n", HTTP_SERVER_IP);
    Socket_Clear(ssource);
    Socket_Write(ssource, boot_buff, strlen(boot_buff));
    // Read answer
    Socket_ClearTimeout(ssource);
    p = boot_buff;
    len_left = BOOT_BUFFER_SIZE;
    while (!Socket_GetTimeout(ssource) && len_left > 0) {
        len = Socket_Read(ssource, p, len_left);
        if (len) {
        	p += len;
        	len_left -= len;
        }
    }
    if ((BOOT_BUFFER_SIZE - len_left) < 10) {
    	return 0; // Err
    }
    return 1; // Ok
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
    SOCKETS_SOURCE ssource;

    // Init sources
    if (LOG_WIFI==1) HAL_UART_Transmit(&huart6, (uint8_t*)"(BOOT Init Sockets)\r\n", 21,100); //Francis, for logging
    Socket_Init(SOCKET_SRC_WIFI);

    Socket_Init(SOCKET_SRC_GPRS);


    // Check connection available first
    // and select source
    // WiFi first

    if (Boot_CheckConnection(SOCKET_SRC_WIFI)) {
    	ssource = SOCKET_SRC_WIFI;
    } else {
    	wlanRecvStop(&huart6);
    	if (Socket_Connect(SOCKET_SRC_GPRS) != SOCKET_OK) {
    		//No connections!
			return BOOT_ERR_CONNECTION; //Err
    	}
    	if (Boot_CheckConnection(SOCKET_SRC_GPRS)) {
        	transport_close(0);
        	Socket_Connect(SOCKET_SRC_GPRS);
    		ssource = SOCKET_SRC_GPRS;
        } else {
        	//No connections!
        	return BOOT_ERR_CONNECTION; //Err
        }
    }



    if (WDT_ENABLED==1)  HAL_IWDG_Refresh(&hiwdg);
/*
	if (Socket_Connect(SOCKET_SRC_GPRS) == SOCKET_OK) {
		if (LOG_WIFI==1) HAL_UART_Transmit(&huart6, "(BOOT Connect GPRS)\r\n", 20,100); //Francis, for logging
    	ssource = SOCKET_SRC_GPRS;
    } else {
    	//No connections!
    	if (LOG_WIFI==1) HAL_UART_Transmit(&huart6, "(BOOT Connect WIFI)\r\n", 20,100); //Francis, for logging
    	return BOOT_ERR_CONNECTION; //Err
    }
*/

	 if (WDT_ENABLED==1)  HAL_IWDG_Refresh(&hiwdg);

    // Clear buffer flash first
	if (LOG_WIFI==1) HAL_UART_Transmit(&huart6, (uint8_t*)"(BOOT Erase Flash_Bank_Copy)\r\n", 29,100); //Francis, for logging
    FlashNVM_EraseBank(FLASH_BANK_COPY);

    if (WDT_ENABLED==1)  HAL_IWDG_Refresh(&hiwdg);

    // Get data
    sprintf(boot_buff, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", HTTP_SERVER_FW_FILENAME, HTTP_SERVER_IP);
    Socket_Clear(ssource);
    Socket_Write(ssource, boot_buff, strlen(boot_buff));
    // Read answer
    Socket_ClearTimeout(ssource);
    total_len = 0;
    while (!Socket_GetTimeout(ssource)) {

        len = Socket_Read(ssource, boot_buff, BOOT_BUFFER_SIZE);
        if (len) {
        	if (WDT_ENABLED==1)  HAL_IWDG_Refresh(&hiwdg);
        	FlashNVM_Write(fl_addr, (uint8_t*)boot_buff, len);
        	total_len += len;
        	fl_addr += len;
        	Socket_ClearTimeout(ssource);
        }
    }
    if (total_len < 10) {
    	//No file on server!
    	if (LOG_WIFI==1) HAL_UART_Transmit(&huart6, (uint8_t*)"(BOOT ERROR No file on server)\r\n", 32,100); //Francis, for logging
    	return BOOT_ERR_CONNECTION; //Err
    }
    if (LOG_WIFI==1) HAL_UART_Transmit(&huart6, (uint8_t*)"(BOOT All data from server is downloaded)\r\n", 42,100); //Francis, for logging
    // Stop HTTP session
    Socket_Close(ssource);

    // NVM flash operation
	 if (WDT_ENABLED==1)  HAL_IWDG_Refresh(&hiwdg);
	// Find firmware length
    fl_addr = FlashNVM_GetBankStartAddress(FLASH_BANK_COPY);
	len = total_len - 24;
    for (i = 0; i < len; i++)
    {
      	if (WDT_ENABLED==1)  HAL_IWDG_Refresh(&hiwdg);
    	FlashNVM_Read(fl_addr + i, (uint8_t*)boot_buff, 24);
    	boot_buff[15] = '\0';
    	if (strstr(boot_buff, "Content-Length:")) {
        	sscanf(boot_buff + 15 + 1, "%ui", &fw_len);
        	fw_len -= 4; //dec CRC
        	break;
    	}
    }
    if (i == len) {
    	//no data length found
    	if (LOG_WIFI==1) HAL_UART_Transmit(&huart6, (uint8_t*)"(BOOT ERROR No length FW valid)\r\n", 32,100); //Francis, for logging
    	return BOOT_ERR_NODATA; //Err
    }

    if (LOG_WIFI==1) HAL_UART_Transmit(&huart6, (uint8_t*)"(BOOT length from firmware is get)\r\n", 36,100); //Francis, for logging

	// Find firmware start position
    fl_addr = FlashNVM_GetBankStartAddress(FLASH_BANK_COPY);
	 if (WDT_ENABLED==1)  HAL_IWDG_Refresh(&hiwdg);

    len = total_len - 4;
    for (i = 0; i < len; i++)
    {
    	if (WDT_ENABLED==1)  HAL_IWDG_Refresh(&hiwdg);
    	FlashNVM_Read(fl_addr + i, (uint8_t*)boot_buff, 4);
    	boot_buff[4] = '\0';
    	p = strstr(boot_buff, "\r\n\r\n");
    	if (p) {
    		fl_addr += (i + 4);
    		break;
    	}
    }
    if (i == len) {
    	//no file found
    	if (LOG_WIFI==1) HAL_UART_Transmit(&huart6, (uint8_t*)"(BOOT ERROR start position FW)\r\n", 32,100); //Francis, for logging
    	return BOOT_ERR_NODATA; //Err
    }

    if (LOG_WIFI==1) HAL_UART_Transmit(&huart6, (uint8_t*)"(BOOT start position in new firmware is get)\r\n", 46,100); //Francis, for logging
    // Count firmware CRC (last 4 byte of the firmware will be - CRC32 checksum)
    crc32_Clear();
	len = 0;
	 if (WDT_ENABLED==1)  HAL_IWDG_Refresh(&hiwdg);
    for (i = 0; i < fw_len; i++)
    {
    	FlashNVM_Read(fl_addr + i, (uint8_t*)boot_buff, 1);
    	crc32_Add(boot_buff[0]);
    }
    crc32 = crc32_Value();
    FlashNVM_Read(fl_addr + i, (uint8_t*)boot_buff, 4);
    // swap order
    boot_buff[4] = boot_buff[0];
    boot_buff[0] = boot_buff[3];
    boot_buff[3] = boot_buff[4];
    boot_buff[4] = boot_buff[1];
    boot_buff[1] = boot_buff[2];
    boot_buff[2] = boot_buff[4];
    // And check it
    if (memcmp(boot_buff, &crc32, 4) != 0) {
    	//Firmware CRC error!
    	if (LOG_WIFI==1) HAL_UART_Transmit(&huart6, (uint8_t*)"(BOOT CRC not valid)\r\n", 22,100); //Francis, for logging
    	return BOOT_ERR_CRC; //Err
    }

    if (LOG_WIFI==1) HAL_UART_Transmit(&huart6, (uint8_t*)"(BOOT CRC is valid)\r\n", 21,100); //Francis, for logging

//	 if (WDT_ENABLED==1)  HAL_IWDG_Refresh(&hiwdg);
    // Check is NEW firmware update available
 //   FlashNVM_Read(fl_addr + APP_VER_ADDR_LOW, (uint8_t*)&len, 2);
 //   FlashNVM_Read(FlashNVM_GetBankStartAddress(FLASH_BANK_APPLICATION) + APP_VER_ADDR_LOW, (uint8_t*)&i, 2);
 //   i   &= 0xFFFF;
 //   len &= 0xFFFF;
 //   if (i != 0xFFFF)
 //   if (i >= len) {
    	//"No new firmware available!");
  //  	if (LOG_WIFI==1) HAL_UART_Transmit(&huart6, "(BOOT No New FW)\r\n", 18,100); //Francis, for logging
   // 	return BOOT_OK;
    //}

    if (LOG_WIFI==1) HAL_UART_Transmit(&huart6, (uint8_t*)"(BOOT version new FW is higher strict than actual FW)\r\n", 55,100); //Francis, for logging
    // Update firmware (copy buffer to Application flash memory)
    FlashNVM_EraseBank(FLASH_BANK_APPLICATION);
    app_addr = FlashNVM_GetBankStartAddress(FLASH_BANK_APPLICATION);
	if (WDT_ENABLED==1)  HAL_IWDG_Refresh(&hiwdg);
    for (i = 0; i < fw_len; i += 128)
    {
    	FlashNVM_Read(fl_addr + i, (uint8_t*)boot_buff, BOOT_BUFFER_SIZE);
    	FlashNVM_Write(app_addr + i, (uint8_t*)boot_buff, BOOT_BUFFER_SIZE);
    }
    // Compare memories again?
    if (LOG_WIFI==1) HAL_UART_Transmit(&huart6, (uint8_t*)"(BOOT updated new FW in Flash_Bank_Application)\r\n", 49,100); //Francis, for logging
    return BOOT_OK;
}


/**
  * @brief  Reboot device via WDT or special command
  * @param  none
  * @retval none
  */
void Boot_RebootMCU(void)
{
    // Reset MCU
    NVIC_SystemReset();
}

