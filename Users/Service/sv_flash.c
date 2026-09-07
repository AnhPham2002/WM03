#include "sv_flash.h"

bool sv_flash_erase(uint32_t u32Address, uint32_t u32Size)
{
    return drv_flash_erase(u32Address, u32Size);
}

bool sv_flash_write(uint32_t u32Address, const uint8_t *pData, uint32_t u32Size)
{
    return drv_flash_write(u32Address, pData, u32Size);
}

void sv_flash_read(uint32_t u32Address, uint8_t *pData, uint32_t u32Size)
{
    drv_flash_read(u32Address, pData, u32Size);
}

uint32_t sv_flash_crc32(uint32_t u32Address, uint32_t u32Size)
{
    return drv_flash_crc32(u32Address, u32Size);
}