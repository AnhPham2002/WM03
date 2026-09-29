#pragma once

#include "drv_gpio.h"

#define LED1_PIN GPIOC, GPIO_PIN_7
#define LED2_PIN GPIOC, GPIO_PIN_9

/**
 * @brief Turn on the LED.
 */
void drv_led_on(void);

/**
 * @brief Turn off the LED.
 */
void drv_led_off(void);

/**
 * @brief Toggle the LED state.
 */
void drv_led_blink(void);