#include "drv_flash.h"
#include "crc.h"

/*==================================================================================================
*                                PRIVATE FUNCTIONS DECLARATIONS
==================================================================================================*/

static inline uint32_t drv_flash_get_page(uint32_t u32Address);

/*==================================================================================================
*                                   PUBLIC FUNCTIONS DEFINITIONS
==================================================================================================*/

bool drv_flash_erase(uint32_t u32Address, uint32_t u32Size)
{
    if ((u32Size == 0) || (u32Address < FLASH_BASE) || (u32Address > FLASH_END) || (u32Size > FLASH_END - u32Address + 1))
    {
        return false;
    }

    uint32_t u32FirstPage = drv_flash_get_page(u32Address);
    uint32_t u32LastAddress = u32Address + u32Size - 1;
    uint32_t u32LastPage = drv_flash_get_page(u32LastAddress);

    FLASH_EraseInitTypeDef sErase = {0};
    uint32_t u32PageError = 0;

    sErase.TypeErase = FLASH_TYPEERASE_PAGES;
    sErase.Banks = FLASH_BANK_1;
    sErase.Page = u32FirstPage;
    sErase.NbPages = (u32LastPage - u32FirstPage) + 1;

    HAL_FLASH_Unlock();

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    HAL_StatusTypeDef eStatus = HAL_FLASHEx_Erase(&sErase, &u32PageError);

    HAL_FLASH_Lock();

    if (eStatus == HAL_OK)
    {
        return true;
    }

    return false;
}

bool drv_flash_write(uint32_t u32Address, const uint8_t *pData, uint32_t u32Size)
{
    if ((u32Size == 0) || (u32Address % 8 != 0))
    {
        return false;
    }

    HAL_StatusTypeDef Status = HAL_OK;
    uint32_t u32Index = 0;

    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    while (u32Index < u32Size)
    {
        uint64_t u64DoubleWord = 0xFFFFFFFFFFFFFFFFULL;

        uint32_t u32Chunk = (u32Size - u32Index >= 8U) ? 8U : (u32Size - u32Index);
        memcpy(&u64DoubleWord, pData + u32Index, u32Chunk);

        Status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, u32Address + u32Index, u64DoubleWord);
        if (Status != HAL_OK)
        {
            break;
        }

        u32Index += 8U;
    }

    HAL_FLASH_Lock();

    if (Status == HAL_OK)
    {
        return true;
    }

    return false;
}

void drv_flash_read(uint32_t u32Address, uint8_t *pData, uint32_t u32Size)
{
    memcpy(pData, (const void *)u32Address, u32Size);
}

uint32_t drv_flash_crc32(uint32_t u32Address, uint32_t u32Size)
{
    const uint32_t *p = (const uint32_t *)u32Address;

    uint32_t u32Crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)p, u32Size);

    return u32Crc;
}

/*==================================================================================================
*                                   PRIVATE FUNCTIONS DEFINITIONS
==================================================================================================*/

static inline uint32_t drv_flash_get_page(uint32_t u32Address)
{
    return (u32Address - FLASH_BASE) / FLASH_PAGE_SIZE;
}