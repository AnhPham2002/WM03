#include "app_main.h"

static uint8_t au8RxTest[1000];
static uint16_t u16RxTestLen;
static uint8_t u8Count = 0;

void app_main(void)
{
    drv_wdt_init();

    sys_debug_init();

    drv_rs485_init();

    drv_4g_init();

    sys_log((const uint8_t *)"System Init\n", 12);

    drv_rs485_pwr_on();

    drv_4g_pwr_on();
    sys_delay_ms(50);
    drv_4g_turn_on();

    while (1)
    {
        drv_wdr_restart();

        sys_log((const uint8_t *)"Hello World\n", 12);
        if (sys_console(au8RxTest, &u16RxTestLen))
        {
        }

        drv_rs485_send((const uint8_t *)"Hello World\n", 12);
        if (drv_rs485_receive(au8RxTest, &u16RxTestLen))
        {
            drv_rs485_send(au8RxTest, u16RxTestLen);
            sys_log(au8RxTest, u16RxTestLen);
        }

        if (drv_4g_receive(au8RxTest, &u16RxTestLen))
        {
            sys_log(au8RxTest, u16RxTestLen);
        }

        if (u8Count == 5)
        {
            u8Count = 0;
            drv_4g_send((const uint8_t *)"AT\r\n", 4);
        }

        drv_led_blink();
        sys_delay_ms(1000);
        u8Count++;
    }
}