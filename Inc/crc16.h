#ifndef __CRC16_H
#define __CRC16_H

#include <stdint.h>

// Public function
void crc16_Clear(void);
void crc32_Clear(void);
uint16_t crc16_Value(void);
uint32_t crc32_Value(void);
void crc16_Add(uint8_t byte);
void crc32_Add(uint8_t byte);

void crc_test(void);

#endif  // __CRC16_H
