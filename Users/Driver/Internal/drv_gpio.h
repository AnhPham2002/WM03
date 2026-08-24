#pragma once

#include "gpio.h"
#include "stm32l431xx.h"
#include "stm32l4xx_hal_gpio.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Write GPIO pin state.
 *
 * @param[in] GPIOx    GPIO port.
 * @param[in] GPIO_Pin GPIO pin.
 * @param[in] PinState Pin state.
 */
void drv_gpio_write(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);

/**
 * @brief Read GPIO pin state.
 *
 * @param[in] GPIOx    GPIO port.
 * @param[in] GPIO_Pin GPIO pin.
 *
 * @return GPIO pin state.
 */
bool drv_gpio_read(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

/**
 * @brief Toggle GPIO pin state.
 *
 * @param[in] GPIOx    GPIO port.
 * @param[in] GPIO_Pin GPIO pin.
 */
void drv_gpio_toggle(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

/**
 * @brief Deinitialize GPIO pin.
 *
 * @param[in] GPIOx    GPIO port.
 * @param[in] GPIO_Pin GPIO pin.
 */
void drv_gpio_deinit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

/**
 * @brief Check and clear pending GPIO interrupt flag.
 *
 * @return true if an interrupt is pending and the flag is cleared,
 *         otherwise false.
 */
bool drv_gpio_irq_pending(void);