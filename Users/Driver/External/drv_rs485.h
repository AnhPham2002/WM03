#pragma once

#include "drv_uart.h"
#include "drv_gpio.h"

#define PWR485_PIN GPIOC, GPIO_PIN_10
#define DE485_PIN GPIOB, GPIO_PIN_4

/**
 * @brief Initialize RS485.
 */
void drv_rs485_init(void);

/**
 * @brief Turn on RS485 power.
 */
void drv_rs485_pwr_on(void);

/**
 * @brief Turn off RS485 power.
 */
void drv_rs485_pwr_off(void);

/**
 * @brief Send data through RS485
 *
 * @param[in] pData   Data buffer.
 * @param[in] u16Size Data size.
 */
void drv_rs485_send(const uint8_t *pData, uint16_t u16Size);

/**
 * @brief Receive data from RS485.
 *
 * @param[out] pData     Receive buffer.
 * @param[in,out] u16Size Data size.
 *
 * @return true if data is received successfully, otherwise false.
 */
bool drv_rs485_receive(uint8_t *pData, uint16_t *u16Size);