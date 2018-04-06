/*
 * crc16.h
 */

#ifndef CRC16_H_
#define CRC16_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*=============================== Includes ===================================*/

#include <stdint.h>

/*=============================== Defines ====================================*/

/*======================= Public functions prototypes ========================*/

uint16_t calcCRC16 (const uint8_t *nData, uint16_t wLength);

/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* CRC16_H_ */
