#include "drv_wdt.h"
#include "drv_gpio.h"
#include "iwdg.h"

void drv_wdt_init(void)
{
    #ifdef USE_WDT
    drv_gpio_write(WDI_PIN, 0);
    #else
    drv_gpio_deinit(WDI_PIN);
    #endif
}

void drv_wdr_restart(void)
{
#ifdef USE_WDT
    drv_gpio_write(WDI_PIN, 1);
    drv_gpio_write(WDI_PIN, 0);

    HAL_IWDG_Refresh(&hiwdg);
#endif
}