#include "app_main.h"

static uint8_t au8RxTest[1000];
static uint16_t u16RxTestLen;

void app_main(void)
{
    drv_wdt_init();

    sys_debug_init();

    drv_rtc_init();

    sys_log((const uint8_t *)"System Init\n", 12);

    while (1)
    {
        sys_log((const uint8_t *)"Hello World\r\n", 14);
        if (sys_console(au8RxTest, &u16RxTestLen))
        {
        }

        memset(au8RxTest, 0, sizeof(au8RxTest));

        drv_wdt_clear_refresh_count();
        sys_delay_ms(1000);
    }
}