#pragma once

#include "drv_uart.h"
#include "drv_gpio.h"

#define CELLULAR_PWR_PIN GPIOC, GPIO_PIN_3
#define CELLULAR_KEY_PIN GPIOB, GPIO_PIN_0
#define CELLULAR_RST_PIN GPIOB, GPIO_PIN_2

#define CELLULAR_TIME_ON 50
#define CELLULAR_TIME_OFF 2500
#define CELLULAR_TIME_RESET 2500
#define CELLULAR_WAIT_FOR_READY 8000

/**
 * @brief Initialize cellular module.
 *
 * Powers off the module, initializes control pins and UART3.
 */
void drv_cellular_init(void);

/**
 * @brief Turn on cellular module power.
 */
void drv_cellular_pwr_on(void);

/**
 * @brief Turn off cellular module power.
 */
void drv_cellular_pwr_off(void);

/**
 * @brief Turn on cellular module.
 *
 * Generates the power key pulse required to turn on the module.
 */
void drv_cellular_turn_on(void);

/**
 * @brief Turn off cellular module.
 *
 * Generates the power key pulse required to turn off the module.
 */
void drv_cellular_turn_off(void);

/**
 * @brief Reset cellular module.
 *
 * Generates the reset pulse required to reset the module.
 */
void drv_cellular_reset(void);

/**
 * @brief Send data to cellular module.
 *
 * @param[in] pData   Data buffer.
 * @param[in] u16Size Data size.
 */
void drv_cellular_send(const uint8_t *pData, uint16_t u16Size);

/**
 * @brief Receive data from cellular module.
 *
 * @param[out] pData      Receive buffer.
 * @param[in,out] u16Size Data size.
 *
 * @return true if data is received successfully, otherwise false.
 */
bool drv_cellular_receive(uint8_t *pData, uint16_t *u16Size);