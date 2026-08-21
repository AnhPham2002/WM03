#pragma once

#include "drv_gpio.h"

#define WKUP_PIN GPIOA, GPIO_PIN_0

/**
 * @brief Detect a rising edge on the wake-up pin.
 *
 * @return true if a rising edge is detected, otherwise false.
 */
bool drv_wkup_pin_rising(void);

/**
 * @brief Detect a falling edge on the wake-up pin.
 *
 * @return true if a falling edge is detected, otherwise false.
 */
bool drv_wkup_pin_falling(void);