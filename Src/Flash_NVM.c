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
	uint8_t sector_start, sectors_n;


	// Check bank to Erase
	if ((fl_bank != FLASH_BANK_APPLICATION) && (fl_bank != FLASH_BANK_COPY)) {
		return HAL_ERROR;
	}

	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
	                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

	if (fl_bank == FLASH_BANK_APPLICATION) {
		sector_start = FLASH_BANKA_START_SECTOR;
		sectors_n = FLASH_BANKA_SECTORS;
	} else {
		sector_start = FLASH_BANKC_START_SECTOR;
		sectors_n = FLASH_BANKC_SECTORS;
	}

	EraseInitStruct.Sector = sector_start;
	EraseInitStruct.TypeErase = TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.NbSectors = sectors_n;

	status = HAL_BUSY;
	while (status == HAL_BUSY) {
		status = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
	}
	HAL_FLASH_Lock();

	return status;
}

/**
  * @brief  erase Application or its copy FLASH
  * @param  fl_bank: flash area (application or its copy bank)
  * @retval true if OK,  otherwise return false
  */
HAL_StatusTypeDef FlashNVM_EraseSector(uint8_t sector)
{
	HAL_StatusTypeDef status;
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t SectorError = 0;
	uint8_t sector_start, sectors_n;



	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
	                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);


	EraseInitStruct.Sector = sector;
	EraseInitStruct.TypeErase = TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.NbSectors = 1;

	status = HAL_BUSY;
	while (status == HAL_BUSY) {
		status = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
	}
	HAL_FLASH_Lock();

	return status;
}


/**
  * @brief  Read a binary array from FLASH
  * @param  address: FLASH relative address to read
  * @param  data_out: output data array pointer
  * @param  size: array length
  * @retval operation status
  */
HAL_StatusTypeDef FlashNVM_Read(uint32_t start_address, uint8_t* data_out, uint32_t size)
{
    uint32_t sizeCounter = 0;

	// Check input data
    if (!IS_FLASH_ADDRESS(start_address)) {
        // It's not Flash's address
    	return HAL_ERROR;
	}

	while (sizeCounter < size) {
	    *data_out = (*(__IO uint32_t*)start_address);
	    data_out++;
	    start_address++;
	    sizeCounter++;
	}
    return 1;
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

	// Check input data
    if (!IS_FLASH_ADDRESS(start_address)) {
        // It's not Flash's address
    	return HAL_ERROR;
	}

	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
	                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

	// Write data
    for (i = 0; i < size; i++) {
    	status = HAL_BUSY;
    	while (status == HAL_BUSY) {
    		status = HAL_FLASH_Program(TYPEPROGRAM_BYTE, start_address + i, data_in[i]);
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
/// Check this for STM32F2xx family
		switch (sect) {
			case FLASH_SECTOR_0:
			case FLASH_SECTOR_1:
			case FLASH_SECTOR_2:
			case FLASH_SECTOR_3:
				size += 16 * 1024; // 16 Kbytes
				break;

			case FLASH_SECTOR_4:
				size += 64 * 1024; // 64 Kbytes
				break;

			default:
				size += 128 * 1024; // 128 Kbytes
				break;
		}
/// End check this for STM32F2xx family. OK, it is same distribution as F4 family. First 4 sectors, 16KB, 4 -> 64, and 5 to 11 -> 128
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
		return FLASH_BASE + FlashNVM_GetSectorSize(FLASH_SECTOR_0, FLASH_BANKB_START_SECTOR) - FlashNVM_GetSectorSize(FLASH_BANKB_START_SECTOR, FLASH_BANKB_START_SECTOR);
	}

	if (fl_bank == FLASH_BANK_APPLICATION) {
		return FLASH_BASE + FlashNVM_GetSectorSize(FLASH_SECTOR_0, FLASH_BANKA_START_SECTOR) - FlashNVM_GetSectorSize(FLASH_BANKA_START_SECTOR, FLASH_BANKA_START_SECTOR);
	}

	if (fl_bank == FLASH_BANK_COPY) {
		return FLASH_BASE + FlashNVM_GetSectorSize(FLASH_SECTOR_0, FLASH_BANKC_START_SECTOR) - FlashNVM_GetSectorSize(FLASH_BANKC_START_SECTOR, FLASH_BANKC_START_SECTOR);
	}
	return 0;
}


int MIC_Flash_Memory_Read(const uint8_t *data_out, uint32_t size)
{
	HAL_StatusTypeDef status = HAL_ERROR;
	uint32_t sizeReceived=0;

	status=FlashNVM_Read(ORIGIN_SECTOR+4, data_out, size); // start to read all data.
	status=FlashNVM_Read(ORIGIN_SECTOR+4, data_out, size); // start to read all data.

	if (status==HAL_ERROR) return -1; // if fails, it returns -1;

	status=FlashNVM_Read(ORIGIN_SECTOR, &sizeReceived, 4); // received quantity of saved bytes.

	if (status==HAL_ERROR) return -1; // if fails, it returns -1;

	if ((status==HAL_OK)&&(sizeReceived==size)) return sizeReceived; // if all goes fine, it returns size of data.


}

int MIC_Flash_Memory_Write(const uint8_t *data_in, uint32_t size)
{


	HAL_StatusTypeDef status = HAL_ERROR;
	uint32_t sizeReceived=0;


	//status=  FlashNVM_EraseSector(FLASH_SECTOR_0);// it erased 16kb at beginning of 0x8000000
	status=  FlashNVM_EraseSector(FLASH_SECTOR_11);// it is erased last flash sector TO CHANGE. A lot of bytes 128kb

	if (status==HAL_ERROR) return -1; // if fails, it returns -1;

	status=FlashNVM_Write(ORIGIN_SECTOR+4, data_in, size); // data are written with +4 offset from starting. First 4 bytes are used to write size of data
																// and watching if there are data or not.
	if (status==HAL_ERROR) return -1; // if fails, it returns -1;

	status=FlashNVM_Write(ORIGIN_SECTOR, &size, 4); // quantity of data are saved at first 4 bytes.

	if (status==HAL_ERROR) return -1; // if fails, it returns -1;

	status=FlashNVM_Read(ORIGIN_SECTOR, &sizeReceived, 4); // quantity of data are read again to verify it wrote good.

	if (status==HAL_ERROR) return -1; // if fails, it returns -1;

	if (status==HAL_OK) return sizeReceived; // if all goes fine, it returns size of data.


}
