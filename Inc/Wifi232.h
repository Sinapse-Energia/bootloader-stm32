#ifndef __WIFI232_H
#define __WIFI232_H

// --- INCLUDES ---
#include <string.h>
#include "stm32f4xx_hal.h"
#include "Definitions.h"

// Public Functions
void wifi_WriteData(UART_HandleTypeDef huart, uint8_t* data_in, int data_len);

#endif // __WIFI232_H
