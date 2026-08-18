#pragma once

#define LED_PIN GPIOA, GPIO_PIN_15

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