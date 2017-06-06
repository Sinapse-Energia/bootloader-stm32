//////////////////////////////////////////////////////////
//
//  Name:
//      FlashMem.c
//
//  Purpose:
//      Read/Write Operation to onboard Flash using ASF NVM library
//
//////////////////////////////////////////////////////////

// --- INCLUDES ---
#include <asf.h>
#include <stdio.h>
#include <string.h>
#include "FlashMem.h"
//#include "Configuration.h"

// --- GLOBAL DATA ASSIGNMENTS and INITIALIZATIONS ---
struct nvm_config nvm_cfg;


//////////////////////////////////////////////////////////////
//
//  Name:
//      FlashMem_Programm
//
//  Purpose:
//      Function for programming data to Flash
//      will check whether the data is greater than Flash page size
//      If it is greater, it splits and writes pagewise
//
//  Inputs:
//      address of the Flash page to be programmed
//      pointer to the buffer containing data to be programmed
//      length of the data to be programmed to Flash
//
//  Outputs:
//      None
//
/////////////////////////////////////////////////////////////////
void FlashMem_Programm(uint32_t address, uint8_t *buffer, uint16_t len)
{
    // --- function body -----
    // Check it first
    if (address < MEM_MAP_EEPROM_ADDR) {
        return;
    }
    if (WORK_MODE == CONF_SLAVE_Mode_Appliaction) {
        if (address >= MEM_MAP_APPLICATION_ADDR) {
            return;
        }
    }

    // Check if length is greater than Flash page size
    if (len > FLASHMEM_PAGE_SIZE) {
        uint32_t offset = 0;

        while (len > FLASHMEM_PAGE_SIZE) {
            // Check if it is first page of a row
            if ((address & 0xFF) == 0) {
                // Erase row
                nvm_erase_row(address);
            }

            // Write one page data to flash
            nvm_write_buffer(address, buffer + offset, FLASHMEM_PAGE_SIZE);
            // Increment the address to be programmed
            address += FLASHMEM_PAGE_SIZE;
            // Increment the offset of the buffer containing data
            offset += FLASHMEM_PAGE_SIZE;
            // Decrement the length
            len -= FLASHMEM_PAGE_SIZE;
        }

        // Check if there is data remaining to be programmed
        if (len > 0) {
            // Write the data to flash
            nvm_write_buffer(address, buffer + offset, len);
        }
    } else {
        // Check if it is first page of a row)
        if ((address & 0xFF) == 0) {
            // Erase row
            nvm_erase_row(address);
        }
        // Write the data to flash
        nvm_write_buffer(address, buffer, len);
    }
} // --- eofFlashMem_Programm( ) ---

//////////////////////////////////////////////////////////////
//
//  Name:
//      FlashMem_ReadPage
//
//  Purpose:
//      Function read one data page array from Flash
//
//  Inputs:
//      address of the Flash page to be readed
//      buffer pointer where data will be stored
//
//  Outputs:
//      output data in buffer
//
/////////////////////////////////////////////////////////////////
void FlashMem_ReadPage(uint16_t page, uint8_t* buf)
{
    // --- local variables ---
    uint32_t addr = (page * FLASHMEM_PAGE_SIZE);
    // --- function body -----
    memcpy(buf, (char*)addr, FLASHMEM_PAGE_SIZE);
} // --- eofFlashMem_ReadPage( ) ---

//////////////////////////////////////////////////////////////
//
//  Name:
//      FlashMem_WritePage
//
//  Purpose:
//      Function for programming single page to Flash
//
//  Inputs:
//      page number to be programmed
//      pointer to the buffer containing data to be programmed
//
//  Outputs:
//      None
//
/////////////////////////////////////////////////////////////////
enum status_code FlashMem_WritePage(uint16_t page, uint8_t* buf)
{
    // --- function body -----
    // Check it first
    if ((page * FLASHMEM_PAGE_SIZE) < MEM_MAP_EEPROM_ADDR) return STATUS_ERR_BAD_ADDRESS;
    if (WORK_MODE == CONF_SLAVE_Mode_Appliaction)
    {
        if ((page * FLASHMEM_PAGE_SIZE) >= MEM_MAP_APPLICATION_ADDR) return STATUS_ERR_BAD_ADDRESS;
    }

    FlashMem_Programm(page * FLASHMEM_PAGE_SIZE, buf, FLASHMEM_PAGE_SIZE);
    return STATUS_OK;
} // --- eofFlashMem_WritePage( ) ---

//////////////////////////////////////////////////////////////
//
//  Name:
//      FlashMem_Init
//
//  Purpose:
//      Main Flash module init function
//      Must be running BRFORE any flash Read/Write operation!
//
//  Inputs:
//      None
//
//  Outputs:
//      None
//
/////////////////////////////////////////////////////////////////
enum status_code FlashMem_Init(void)
{
    // --- function body -----
    nvm_get_config_defaults(&nvm_cfg);
    nvm_cfg.manual_page_write = false;
    return nvm_set_config(&nvm_cfg);
} // --- eofFlashMem_Init( ) ---

//////////////////////////////////////////////////////////////
//
//  Name:
//      FlashMem_ReadByte
//
//  Purpose:
//      Function read one byte from Flash
//
//  Inputs:
//      address of the Flash to be readed
//
//  Outputs:
//      output data
//
/////////////////////////////////////////////////////////////////
uint8_t FlashMem_ReadByte(uint32_t addr)
{
    // --- local variables ---
    char* pointer = (char*)addr;
    // --- function body -----
    return *pointer;
} // --- eofFlashMem_ReadByte( ) ---

//////////////////////////////////////////////////////////////
//
//  Name:
//      FlashMem_WriteByte
//
//  Purpose:
//      Function for programming one data byte to Flash
//
//  Inputs:
//      address of the Flash to be programmed
//      data to be programmed
//
//  Outputs:
//      Write satatus code
//
/////////////////////////////////////////////////////////////////
enum status_code FlashMem_WriteByte(uint32_t addr, uint8_t data)
{
    // --- function body -----
    // Check it first
    if (addr < MEM_MAP_EEPROM_ADDR) return STATUS_ERR_BAD_ADDRESS;
    if (WORK_MODE == CONF_SLAVE_Mode_Appliaction)
    {
        if (addr >= MEM_MAP_APPLICATION_ADDR) return STATUS_ERR_BAD_ADDRESS;
    }

    return nvm_update_buffer(addr, &data, addr % FLASHMEM_PAGE_SIZE, 1);
} // --- eofFlashMem_WriteByte( ) ---

//////////////////////////////////////////////////////////////
//
//  Name:
//      FlashMem_ReadWord
//
//  Purpose:
//      Function read one word from Flash
//
//  Inputs:
//      address of the Flash to be readed
//
//  Outputs:
//      output data
//
/////////////////////////////////////////////////////////////////
uint16_t FlashMem_ReadWord(uint32_t addr)
{
    // --- local variables ---
    uint16_t out;

    // --- function body -----
    out  = FlashMem_ReadByte(addr + 1);
    out <<= 8;
    out |= FlashMem_ReadByte(addr);

    return out;
} // --- eof( ) ---

//////////////////////////////////////////////////////////////
//
//  Name:
//      FlashMem_WriteWord
//
//  Purpose:
//      Function for programming one word to Flash
//
//  Inputs:
//      address of the Flash to be programmed
//      data to be programmed
//
//  Outputs:
//      None
//
/////////////////////////////////////////////////////////////////
enum status_code FlashMem_WriteWord(uint32_t addr, uint16_t data)
{
    // --- function body -----
    FlashMem_WriteByte(addr + 1, data >> 8);
    return FlashMem_WriteByte(addr, data);
} // --- eofFlashMem_WriteWord( ) ---

//////////////////////////////////////////////////////////////
//
//  Name:
//      FlashMem_ReadDWord
//
//  Purpose:
//      Function read double words from Flash
//
//  Inputs:
//      address of the Flash to be readed
//
//  Outputs:
//      output data
//
/////////////////////////////////////////////////////////////////
uint32_t FlashMem_ReadDWord(uint32_t addr)
{
    // --- local variables ---
    uint32_t out;

    // --- function body -----
    out  = FlashMem_ReadWord(addr + 2);
    out <<= 16;
    out |= FlashMem_ReadWord(addr);

    return out;
} // --- eofFlashMem_ReadDWord( ) ---

//////////////////////////////////////////////////////////////
//
//  Name:
//      FlashMem_WriteDWord
//
//  Purpose:
//      Function for programming double words to Flash
//
//  Inputs:
//      address of the Flash to be programmed
//      data to be programmed
//
//  Outputs:
//      None
//
/////////////////////////////////////////////////////////////////
enum status_code FlashMem_WriteDWord(uint32_t addr, uint32_t data)
{
    // --- function body -----
    FlashMem_WriteWord(addr + 2, data >> 16);
    return FlashMem_WriteWord(addr, data);
} // --- eofFlashMem_WriteDWord( ) ---

//////////////////////////////////////////////////////////////
//
//  Name:
//      FlashMem_CountCrc
//
//  Purpose:
//      Recount Flash memory CRC
//
//  Inputs:
//      Start and stop Flash address for CRC calculation
//
//  Outputs:
//      Two byte CRC value
//
/////////////////////////////////////////////////////////////////
uint16_t FlashMem_CountCrc(uint32_t addr_start, uint32_t addr_stop)
{
    // --- local variables ---
    static const uint16_t magic = 0xAC4F;
    uint16_t crc16 = 0;
    uint32_t cnt = 0;

    // --- function body -----
    for (cnt = addr_start; cnt <= addr_stop; cnt++) {
        crc16 += FlashMem_ReadByte(cnt) * magic;
    }

    return crc16;
} // --- eofFlashMem_CountCrc( ) ---