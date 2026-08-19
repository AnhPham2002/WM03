#include "app_main.h"

static uint8_t au8RxTest[1000];
static uint16_t u16RxTestLen;

void app_main(void)
{
    drv_wdt_init();

    sys_debug_init();

    sys_log((const uint8_t *)"System Init\n", 12);

    while (1)
    {
        drv_wdr_restart();

        sys_log((const uint8_t *)"Hello World\n", 12);
        if (sys_console(au8RxTest, &u16RxTestLen))
        {
        }

        drv_led_blink();
        sys_delay_ms(1000);
    }
}