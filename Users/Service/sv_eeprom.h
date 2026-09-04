#pragma once

#include "drv_eeprom.h"

#define MAX_READ_RETRY 3
#define MAX_WRITE_RETRY 3
#define EEPROM_BUFFER_SIZE 512

/**
 * @brief Initialize eeprom service.
 */
void sv_eeprom_init(void);

/**
 * @brief Read data from eeprom with CRC16 verification and retry.
 *
 * @param[in]  u32Address Start address.
 * @param[out] pData      Data buffer.
 * @param[in]  u32Size    Data size.
 *
 * @return true if data is read successfully, otherwise false.
 */
bool sv_eeprom_read(uint32_t u32Address, uint8_t *pData, uint32_t u32Size);

/**
 * @brief Write data to eeprom with CRC16 verification and retry.
 *
 * @param[in] u32Address Start address.
 * @param[in] pData      Data buffer.
 * @param[in] u32Size    Data size.
 *
 * @return true if data is written and verified successfully, otherwise false.
 */
bool sv_eeprom_write(uint32_t u32Address, const uint8_t *pData, uint32_t u32Size);