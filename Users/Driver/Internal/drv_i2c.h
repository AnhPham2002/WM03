#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "stm32l4xx_hal.h"

#define I2C_POLLING_TIMEOUT 100

/**
 * @brief Initialize I2C.
 */
void drv_i2c_init(void);

/**
 * @brief Deinitialize I2C.
 */
void drv_i2c_deinit(void);

/**
 * @brief Send data through I2C.
 *
 * @param[in] pData   Data buffer.
 * @param[in] u16Size Data size.
 *
 * @return true if data is send successfully, otherwise false.
 */
bool drv_i2c_send(uint16_t u16DevAddr, uint8_t *pData, uint16_t u16Size);

/**
 * @brief Receive data from I2C.
 *
 * @param[out] pData     Receive buffer.
 * @param[in,out] u16Size Data size.
 *
 * @return true if data is received successfully, otherwise false.
 */
bool drv_i2c_receive(uint16_t u16DevAddr, uint8_t *pData, uint16_t u16Size);