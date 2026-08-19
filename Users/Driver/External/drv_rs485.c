#include "drv_rs485.h"
#include "drv_gpio.h"
#include "drv_uart.h"

/**
 * @brief Set RS485 to transmit mode.
 */
static inline void drv_rs485_set_mode_tx(void);

/**
 * @brief Set RS485 to receive mode.
 */
static inline void drv_rs485_set_mode_rx(void);

void drv_rs485_init(void)
{
    drv_rs485_pwr_off();
    drv_rs485_set_mode_rx();
    drv_uart1_init(19200, UART_PARITY_EVEN);
}

void drv_rs485_pwr_on(void)
{
    drv_gpio_write(PWR485_PIN, 0);
}

void drv_rs485_pwr_off(void)
{
    drv_gpio_write(PWR485_PIN, 1);
}

bool drv_rs485_send(const uint8_t *pData, uint16_t u16Size)
{
    bool bTxStatus = false;
    drv_rs485_set_mode_tx();
    bTxStatus = drv_uart1_send(pData, u16Size);
    drv_rs485_set_mode_rx();
    return bTxStatus;
}

bool drv_rs485_receive(uint8_t *pData, uint16_t *u16Size)
{
    return drv_uart1_receive(pData, u16Size);
}

static inline void drv_rs485_set_mode_tx(void)
{
    drv_gpio_write(DE485_PIN, 1);
}

static inline void drv_rs485_set_mode_rx(void)
{
    drv_gpio_write(DE485_PIN, 0);
}