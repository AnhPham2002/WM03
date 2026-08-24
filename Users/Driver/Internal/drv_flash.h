#pragma once

#include "stm32l4xx_hal.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief Erase flash memory.
 *
 * @param[in] u32Address Start address.
 * @param[in] u32Size    Size to erase.
 *
 * @return true if successful, otherwise false.
 */
bool drv_flash_erase(uint32_t u32Address, uint32_t u32Size);

/**
 * @brief Write data to flash memory.
 *
 * @param[in] u32Address Start address.
 * @param[in] pData      Data buffer.
 * @param[in] u32Size    Data size.
 *
 * @return true if successful, otherwise false.
 */
bool drv_flash_write(uint32_t u32Address, const uint8_t *pData, uint32_t u32Size);

/**
 * @brief Read data from flash memory.
 *
 * @param[in]  u32Address Start address.
 * @param[out] pData      Data buffer.
 * @param[in]  u32Size    Data size.
 */
void drv_flash_read(uint32_t u32Address, uint8_t *pData, uint32_t u32Size);

/**
 * @brief Calculate CRC32 of flash data.
 *
 * @param[in] u32Address Start address.
 * @param[in] u32Size    Data size.
 *
 * @return CRC32 value.
 */
uint32_t drv_flash_crc32(uint32_t u32Address, uint32_t u32Size);