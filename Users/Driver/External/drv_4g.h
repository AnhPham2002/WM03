#pragma once

#include "drv_uart.h"
#include "drv_gpio.h"

#define PWR4G_PIN GPIOC, GPIO_PIN_3
#define KEY4G_PIN GPIOB, GPIO_PIN_0
#define RST4G_PIN GPIOB, GPIO_PIN_2

#define MODULE_TIME_ON 50
#define MODULE_TIME_OFF 2500
#define MODULE_TIME_RESET 2500
#define MODULE_WAIT_FOR_READY 8000

/**
 * @brief Initialize 4G module.
 */
void drv_4g_init(void);

/**
 * @brief Turn on 4G module power.
 */
void drv_4g_pwr_on(void);

/**
 * @brief Turn off 4G module power.
 */
void drv_4g_pwr_off(void);

/**
 * @brief Turn on 4G module.
 */
void drv_4g_turn_on(void);

/**
 * @brief Turn off 4G module.
 */
void drv_4g_turn_off(void);

/**
 * @brief Reset 4G module.
 */
void drv_4g_reset(void);

/**
 * @brief Send data to 4G module.
 *
 * @param[in] pData   Data buffer.
 * @param[in] u16Size Data size.
 */
void drv_4g_send(const uint8_t *pData, uint16_t u16Size);

/**
 * @brief Receive data from 4G module.
 *
 * @param[out] pData     Receive buffer.
 * @param[in,out] u16Size Data size.
 *
 * @return true if data is received successfully, otherwise false.
 */
bool drv_4g_receive(uint8_t *pData, uint16_t *u16Size);