#include "sv_eeprom.h"

static uint8_t au8EepromBuf[EEPROM_BUFFER_SIZE];

/*==================================================================================================
*                                PRIVATE FUNCTIONS DECLARATIONS
==================================================================================================*/

/**
 * @brief Read a data record from EEPROM and verify CRC16.
 *
 * @param[in]  u32Address Start address.
 * @param[out] pData      Data buffer.
 * @param[in]  u32Size    Data size.
 *
 * @return true if the record is read and CRC16 is valid, otherwise false.
 */
static bool sv_eeprom_read_record(uint32_t u32Address, uint8_t *pData, uint32_t u32Size);

/**
 * @brief Write a data record to EEPROM with CRC16.
 *
 * @param[in] u32Address Start address.
 * @param[in] pData      Data buffer.
 * @param[in] u32Size    Data size.
 *
 * @return true if the record is written successfully, otherwise false.
 */
static bool sv_eeprom_write_record(uint32_t u32Address, const uint8_t *pData, uint32_t u32Size);

/*==================================================================================================
*                                   PUBLIC FUNCTIONS DEFINITIONS
==================================================================================================*/

void sv_eeprom_init(void)
{
    drv_eeprom_init();
}

bool sv_eeprom_read(uint32_t u32Address, uint8_t *pData, uint32_t u32Size)
{
    uint8_t u8RetryIndex = 0;

    while (u8RetryIndex < MAX_READ_RETRY)
    {
        if (sv_eeprom_read_record(u32Address, pData, u32Size))
        {
            return true;
        }
        u8RetryIndex++;
    }

    return false;
}

bool sv_eeprom_write(uint32_t u32Address, const uint8_t *pData, uint32_t u32Size)
{
    uint8_t u8RetryIndex = 0;

    while (u8RetryIndex < MAX_WRITE_RETRY)
    {
        if (sv_eeprom_write_record(u32Address, pData, u32Size))
        {
            if (sv_eeprom_read(u32Address, au8EepromBuf, u32Size))
            {
                if (memcmp(au8EepromBuf, pData, u32Size) == 0)
                {
                    return true;
                }
            }
        }

        u8RetryIndex++;
    }

    return false;
}

/*==================================================================================================
*                                   PRIVATE FUNCTIONS DEFINITIONS
==================================================================================================*/

static bool sv_eeprom_read_record(uint32_t u32Address, uint8_t *pData, uint32_t u32Size)
{
    uint8_t au8CrcBuf[2];

    // Read (data + CRC16) from EEPROM
    if ((!drv_eeprom_read_data(u32Address, pData, u32Size)) || (!drv_eeprom_read_data(u32Address + u32Size, au8CrcBuf, 2)))
    {
        return false;
    }

    // Extract CRC16 from the last two bytes: Low byte first, High byte next
    uint16_t u16Crc = (uint16_t)(au8CrcBuf[0]) | (uint16_t)(au8CrcBuf[1] << 8);

    // Verify CRC16 integrity against calculated value
    if (u16Crc != sys_crc16(pData, (uint16_t)u32Size))
    {
        return false; // Data corrupted
    }

    return true; // Data is valid
}

static bool sv_eeprom_write_record(uint32_t u32Address, const uint8_t *pData, uint32_t u32Size)
{
    // Compute CRC16 for the data block
    uint16_t u16Crc = sys_crc16(pData, (uint16_t)u32Size);

    // Append CRC16 to buffer: Low byte first
    uint8_t au8CrcBuf[2];
    au8CrcBuf[0] = (uint8_t)(u16Crc & 0xFF);        // Low byte
    au8CrcBuf[1] = (uint8_t)((u16Crc >> 8) & 0xFF); // High byte

    // Write (data + 2 CRC bytes) to EEPROM
    if ((!drv_eeprom_write_data(u32Address, pData, u32Size)) || (!drv_eeprom_write_data(u32Address + u32Size, au8CrcBuf, 2)))
    {
        return false; // Write failed
    }

    return true; // Write succeeded
}