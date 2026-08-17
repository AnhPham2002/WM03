#include "app_main.h"
#include "gpio.h"
#include "iwdg.h"

void app_main(void)
{
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_15);
    HAL_Delay(1000);
    HAL_IWDG_Refresh(&hiwdg);
}