#include "drv_led.h"
#include "drv_gpio.h"

void drv_led_blink(void)
{
    drv_gpio_toggle(LED_PIN);
}