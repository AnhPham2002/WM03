#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "stm32l4xx_hal.h"

#define SPI_POLLING_TIMEOUT 100

/**
 * @brief Initialize SPI.
 */
void drv_spi_init(void);

/**
 * @brief Deinitialize SPI.
 */
void drv_spi_deinit(void);

/**
 * @brief Send data through SPI.
 *
 * @param[in] pData   Data buffer.
 * @param[in] u16Size Data size.
 *
 * @return true if data is send successfully, otherwise false.
 */
bool drv_spi_send(const uint8_t *pData, uint16_t u16Size);

/**
 * @brief Receive data from SPI.
 *
 * @param[out] pData     Receive buffer.
 * @param[in,out] u16Size Data size.
 *
 * @return true if data is received successfully, otherwise false.
 */
bool drv_spi_receive(uint8_t *pData, uint16_t u16Size);