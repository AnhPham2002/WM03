#include "drv_wdt.h"
#include "drv_gpio.h"
#include "iwdg.h"

static volatile uint32_t u32WdtRefreshCount;

void drv_wdt_init(void)
{
#ifdef USE_WDT
    drv_gpio_write(WDI_PIN, 0);
#else
    drv_gpio_deinit(WDI_PIN);
#endif
}

void drv_wdt_refresh(void)
{
#ifdef USE_WDT
    if (u32WdtRefreshCount++ <= WDT_MAX_REFRESH_COUNT)
    {
        drv_gpio_write(WDI_PIN, 1);
        drv_gpio_write(WDI_PIN, 0);

        HAL_IWDG_Refresh(&hiwdg);
    }
#endif
}

void drv_wdt_clear_refresh_count(void)
{
    u32WdtRefreshCount = 0;
}