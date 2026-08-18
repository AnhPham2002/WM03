#include "app_main.h"
#include "drv_common.h"
#include "drv_led.h"
#include "drv_wdt.h"

void app_main(void)
{
    drv_wdt_init();

    while (1)
    {
        drv_wdr_restart();

        drv_led_blink();
        delay_ms(1000);
    }
}