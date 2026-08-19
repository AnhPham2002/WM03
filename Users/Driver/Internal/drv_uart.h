#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "stm32l4xx_hal.h"

#include "sys_common.h"

#define UART1_RX_BUFFER_SIZE 2000
#define UART2_RX_BUFFER_SIZE 50
#define UART3_RX_BUFFER_SIZE 2000
#define UART_POLLING_TIMEOUT 500
#define UART_RX_FRAME_TIMEOUT 1000

void drv_uart1_init(uint32_t u32Baud, uint32_t u32Parity);
void drv_uart1_deinit(void);
void drv_uart1_send(const uint8_t *pData, uint16_t u16Size);
bool drv_uart1_receive(uint8_t *pData, uint16_t *u16Size);

void drv_uart2_init(uint32_t u32Baud, uint32_t u32Parity);
void drv_uart2_deinit(void);
void drv_uart2_send(const uint8_t *pData, uint16_t u16Size);
bool drv_uart2_receive(uint8_t *pData, uint16_t *u16Size);

void drv_uart3_init(uint32_t u32Baud, uint32_t u32Parity);
void drv_uart3_deinit(void);
void drv_uart3_send(const uint8_t *pData, uint16_t u16Size);
bool drv_uart3_receive(uint8_t *pData, uint16_t *u16Size);