#include "drv_led.h"

void drv_led_on(void)
{
    drv_gpio_write(LED1_PIN, 0);
    drv_gpio_write(LED2_PIN, 0);
}

void drv_led_off(void)
{
    drv_gpio_write(LED1_PIN, 1);
    drv_gpio_write(LED2_PIN, 1);
}

void drv_led_blink(void)
{
    drv_gpio_toggle(LED1_PIN);
    drv_gpio_toggle(LED2_PIN);
}