#include <Crc32.h>

// Global vars
uint32_t crc32_value;


// CRC-32C (iSCSI) polynomial in reversed bit order.
//#define POLY 0x82f63b78
// CRC-32 (Ethernet, ZIP, etc.) polynomial in reversed bit order.
#define POLY 0xedb88320

/**
  * @brief  Set CRC initial value
  * @param  none
  * @retval none
  */
void crc32_Clear(void)
{
	crc32_value = 0x00000000;
}

/**
  * @brief  Return current CRC value
  * @param  none
  * @retval 32-bit CRC
  */
uint32_t crc32_Value(void)
{
	return crc32_value;
}

/**
  * @brief  Recount CRC32 cycle (add new value)
  * @param  byte: data byte
  * @retval none
  */
void crc32_Add(uint8_t byte)
{
	uint32_t crc = ~crc32_value;

	crc ^= byte;
	for (unsigned int j = 0; j < 8; j++)
		if (crc & 1)
			crc = (crc >> 1) ^ POLY;
		else
			crc =  crc >> 1;

	crc32_value = ~crc;
}
