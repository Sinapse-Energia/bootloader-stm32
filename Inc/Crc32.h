#ifndef __CRC16_H
#define __CRC16_H

// --- INCLUDES ---
#include <stdint.h>

// Public function
void crc32_Clear(void);
uint32_t crc32_Value(void);
void crc32_Add(uint8_t byte);

#endif  // __CRC16_H
