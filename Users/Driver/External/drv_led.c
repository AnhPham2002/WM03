#include "drv_led.h"
#include "drv_gpio.h"

void drv_led_on(void)
{
    drv_gpio_write(LED_PIN, 0);
}

void drv_led_off(void)
{
    drv_gpio_write(LED_PIN, 1);
}

void drv_led_blink(void)
{
    drv_gpio_toggle(LED_PIN);
}