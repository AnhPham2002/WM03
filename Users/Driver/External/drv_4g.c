#include "drv_4g.h"
#include "drv_uart.h"

void drv_4g_init(void)
{
    drv_4g_pwr_off();
    drv_gpio_write(KEY4G_PIN, 0);
    drv_gpio_write(RST4G_PIN, 0);
    drv_uart3_init(115200, UART_PARITY_NONE);
}

void drv_4g_pwr_on(void)
{
    drv_gpio_write(PWR4G_PIN, 1);
}

void drv_4g_pwr_off(void)
{
    drv_gpio_write(PWR4G_PIN, 0);
}

void drv_4g_turn_on(void)
{
    drv_gpio_write(KEY4G_PIN, 1);
    sys_delay_ms(MODULE_TIME_ON);
    drv_gpio_write(KEY4G_PIN, 0);
}

void drv_4g_turn_off(void)
{
    drv_gpio_write(KEY4G_PIN, 1);
    sys_delay_ms(MODULE_TIME_OFF);
    drv_gpio_write(KEY4G_PIN, 0);
}

void drv_4g_reset(void)
{
    drv_gpio_write(RST4G_PIN, 1);
    sys_delay_ms(MODULE_TIME_RESET);
    drv_gpio_write(RST4G_PIN, 0);
}

void drv_4g_send(const uint8_t *pData, uint16_t u16Size)
{
    drv_uart3_send(pData, u16Size);
}

bool drv_4g_receive(uint8_t *pData, uint16_t *u16Size)
{
    return drv_uart3_receive(pData, u16Size);
}