#include "Flash_NVM.h"


/**
  * @brief  erase Application or its copy FLASH
  * @param  fl_bank: flash area (application or its copy bank)
  * @retval true if OK,  otherwise return false
  */
HAL_StatusTypeDef FlashNVM_EraseBank(FLASH_BANK fl_bank)
{
	HAL_StatusTypeDef status;
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t SectorError = 0;
	uint8_t page_start, pages_n;


	// Check bank to Erase
	if ((fl_bank != FLASH_BANK_APPLICATION) && (fl_bank != FLASH_BANK_COPY)) {
		return HAL_ERROR;
	}

	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR );

	if (fl_bank == FLASH_BANK_APPLICATION) {
		page_start = FLASH_BANKA_START_PAGE;
		pages_n = FLASH_BANKA_PAGES;
	} else {
		page_start = FLASH_BANKC_START_PAGE;
		pages_n = FLASH_BANKC_PAGES;
	}


	EraseInitStruct.PageAddress=page_start;
	EraseInitStruct.NbPages=pages_n;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;


	status = HAL_BUSY;
	while (status == HAL_BUSY) {
			status = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
	}
	HAL_FLASH_Lock();

	return status;

}




HAL_StatusTypeDef FlashNVM_Read(uint32_t start_address, uint8_t* data_out, uint32_t size)
{
    uint32_t sizeCounter = 0;


	// Check input data
   // if (!IS_FLASH_ADDRESS(start_address)) {
        // It's not Flash's address
    //	return HAL_ERROR;
	//}

	while (sizeCounter < size) {
	    *data_out = (*(__IO uint8_t*)start_address);
	    data_out++;
	    start_address++;
	    sizeCounter++;
	}
    return HAL_OK;
}


/**
  * @brief  write data array to PREVIOSLY ERISED FLASH memory
  * @param  fl_bank: flash area (application or its copy bank)
  * @retval true if OK,  otherwise return false
  */
HAL_StatusTypeDef FlashNVM_Write(uint32_t start_address, const uint8_t* data_in, uint32_t size)
{
	HAL_StatusTypeDef status = HAL_ERROR;
	uint32_t i;
	uint16_t *integerPointer;
	integerPointer = (uint16_t *)data_in;

	// Check input data
    //if (!IS_FLASH_ADDRESS(start_address)) {
        // It's not Flash's address
    //	return HAL_ERROR;
	//}

	HAL_FLASH_Unlock();
	//__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
	//                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR );


	// Write data
    for (i = 0; i < size/2; i+=1) {
    	status = HAL_BUSY;
    	while (status == HAL_BUSY) {
    		status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, start_address + 2*i, *integerPointer);
    		integerPointer++;
    	}
    	if ( status != HAL_OK) {
    		break;
    	}
    }

	HAL_FLASH_Lock();

	return status;
}




/**
  * @brief  Count Sectors data size
  * @param  start_sector: start FLASH sector
  * @param  last_sector: last FLASH area sector
  * @retval overall data size in bytes
  */
uint32_t FlashNVM_GetSectorSize(uint8_t start_sector, uint8_t last_sector)
{
	uint32_t size = 0;
	uint8_t sect;

	for (sect = start_sector; sect <= last_sector; sect++)
	{

		size += 4 * 1024; //4 Kbytes For STM32F0xx devices

	}
	return size;
}



/**
  * @brief  Count selected data bank size
  * @param  fl_bank: flash area (bootloader/ application or its copy bank)
  * @retval overall data size in bytes
  */
uint32_t FlashNVM_GetBankSize(FLASH_BANK fl_bank)
{

	if (fl_bank == FLASH_BANK_BOOTLOADER) {
			return FlashNVM_GetSectorSize(FLASH_BANKB_START_SECTOR, FLASH_BANKB_START_SECTOR + FLASH_BANKB_SECTORS - 1);
	}

	if (fl_bank == FLASH_BANK_APPLICATION) {
			return FlashNVM_GetSectorSize(FLASH_BANKA_START_SECTOR, FLASH_BANKA_SECTORS + FLASH_BANKB_SECTORS - 1);
	}

	if (fl_bank == FLASH_BANK_COPY) {
			return FlashNVM_GetSectorSize(FLASH_BANKC_START_SECTOR, FLASH_BANKC_SECTORS + FLASH_BANKB_SECTORS - 1);
	}

	return 0;
}


/**
  * @brief  Count selected data bank start address
  * @param  fl_bank: flash area (bootloader/ application or its copy bank)
  * @retval start address value in FLASH
  */
uint32_t FlashNVM_GetBankStartAddress(FLASH_BANK fl_bank)
{
	if (fl_bank == FLASH_BANK_BOOTLOADER) {
		return FLASH_BASE + FlashNVM_GetSectorSize(0, FLASH_BANKB_START_SECTOR) - FlashNVM_GetSectorSize(FLASH_BANKB_START_SECTOR, FLASH_BANKB_START_SECTOR);
	}

	if (fl_bank == FLASH_BANK_APPLICATION) {
		return FLASH_BASE + FlashNVM_GetSectorSize(0, FLASH_BANKA_START_SECTOR) - FlashNVM_GetSectorSize(FLASH_BANKA_START_SECTOR, FLASH_BANKA_START_SECTOR);
	}

	if (fl_bank == FLASH_BANK_COPY) {
		return FLASH_BASE + FlashNVM_GetSectorSize(0, FLASH_BANKC_START_SECTOR) - FlashNVM_GetSectorSize(FLASH_BANKC_START_SECTOR, FLASH_BANKC_START_SECTOR);
	}
	return 0;
}














