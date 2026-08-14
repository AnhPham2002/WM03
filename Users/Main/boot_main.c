#include "boot_main.h"
#include "gpio.h"
#include "stm32l4xx_hal.h"

void led_blink(void)
{
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_15);
    HAL_Delay(1000);
}