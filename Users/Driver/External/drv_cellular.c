#include "drv_cellular.h"
#include "drv_uart.h"

void drv_cellular_init(void)
{
    drv_cellular_pwr_off();
    drv_gpio_write(CELLULAR_KEY_PIN, 0);
    drv_gpio_write(CELLULAR_RST_PIN, 0);
    drv_uart3_init(115200, UART_PARITY_NONE);
}

void drv_cellular_pwr_on(void)
{
    drv_gpio_write(CELLULAR_PWR_PIN, 1);
}

void drv_cellular_pwr_off(void)
{
    drv_gpio_write(CELLULAR_PWR_PIN, 0);
}

void drv_cellular_turn_on(void)
{
    drv_gpio_write(CELLULAR_KEY_PIN, 1);
    sys_delay_ms(CELLULAR_TIME_ON);
    drv_gpio_write(CELLULAR_KEY_PIN, 0);
}

void drv_cellular_turn_off(void)
{
    drv_gpio_write(CELLULAR_KEY_PIN, 1);
    sys_delay_ms(CELLULAR_TIME_OFF);
    drv_gpio_write(CELLULAR_KEY_PIN, 0);
}

void drv_cellular_reset(void)
{
    drv_gpio_write(CELLULAR_RST_PIN, 1);
    sys_delay_ms(CELLULAR_TIME_RESET);
    drv_gpio_write(CELLULAR_RST_PIN, 0);
}

void drv_cellular_send(const uint8_t *pData, uint16_t u16Size)
{
    drv_uart3_send(pData, u16Size);
}

bool drv_cellular_receive(uint8_t *pData, uint16_t *u16Size)
{
    return drv_uart3_receive(pData, u16Size);
}