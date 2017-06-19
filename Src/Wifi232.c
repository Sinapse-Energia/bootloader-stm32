#include <Wifi232.h>

/**
  * @brief  Send a data throw USR-WIFI232 module using
  *  		'serial command mode'
  * @param  str_in: string to send
  * @retval none
  */
void wifi_WriteData(UART_HandleTypeDef huart, uint8_t* data_in, int data_len)
{
	uint8_t i, crc = 0x00;
	uint16_t length;
	uint8_t tx[4];

	// Send header 2b
	tx[0] = 0x55;
	tx[1] = 0xAA;
	HAL_UART_Transmit(&huart, tx, 2, 1000);

	// Send total length  2b (do not contain last 1b CRC)
	// High byte on the front!
	length = data_len + 4 + 5;
	tx[0] = (length >> 8) & 0xFF;
	tx[1] = length & 0xFF;
	HAL_UART_Transmit(&huart, tx, 2, 1000);

	// Function byte
	// Bit0: (UDP:0 TCP:1)
	// Bit1: (Short connection:0 Long connection:1)
	// Bit2: (IP:0 Domain name:1)
	// ...
	// Bit7: 0 (cut protocol only support)
	tx[0] = 0x03;
	crc += tx[0];
	HAL_UART_Transmit(&huart, tx, 1, 1000);

	// Backup data area
	// 0x00 0x00 for long connection
	tx[0] = 0x00;
	tx[1] = 0x00;
	crc += tx[0];
	crc += tx[1];
	HAL_UART_Transmit(&huart, tx, 2, 1000);

	// Destination port 2b
	tx[0] = HTTP_SERVER_PORT & 0xFF;
	tx[1] = (HTTP_SERVER_PORT >> 8) & 0xFF;
	crc += tx[0];
	crc += tx[1];
	HAL_UART_Transmit(&huart, tx, 2, 1000);

	// Target address 4b
	tx[0] = HTTP_SERVER_IP_00;
	tx[1] = HTTP_SERVER_IP_01;
	tx[2] = HTTP_SERVER_IP_02;
	tx[3] = HTTP_SERVER_IP_03;
	crc += tx[0];
	crc += tx[1];
	crc += tx[2];
	crc += tx[3];
	HAL_UART_Transmit(&huart, tx, 4, 1000);

	// Send data N-bytes
	for (i = 0; i < data_len; i++) {
		crc += data_in[i];
	}
	HAL_UART_Transmit(&huart, data_in, data_len, 1000);

	// Sent CRC 1b
	tx[0] = crc;
	HAL_UART_Transmit(&huart, tx, 1, 1000);
}
