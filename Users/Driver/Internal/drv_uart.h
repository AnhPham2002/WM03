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

//------------------------------------------------------------------------------
// UART1
//------------------------------------------------------------------------------

/**
 * @brief Initialize UART1.
 *
 * @param[in] u32Baud   Baud rate.
 * @param[in] u32Parity Parity configuration.
 */
void drv_uart1_init(uint32_t u32Baud, uint32_t u32Parity);

/**
 * @brief Deinitialize UART1.
 */
void drv_uart1_deinit(void);

/**
 * @brief Send data through UART1.
 *
 * @param[in] pData   Data buffer.
 * @param[in] u16Size Data size.
 */
void drv_uart1_send(const uint8_t *pData, uint16_t u16Size);

/**
 * @brief Receive data from UART1.
 *
 * @param[out] pData     Receive buffer.
 * @param[in,out] u16Size Data size.
 *
 * @return true if data is received successfully, otherwise false.
 */
bool drv_uart1_receive(uint8_t *pData, uint16_t *u16Size);


//------------------------------------------------------------------------------
// UART2
//------------------------------------------------------------------------------

/**
 * @brief Initialize UART2.
 *
 * @param[in] u32Baud   Baud rate.
 * @param[in] u32Parity Parity configuration.
 */
void drv_uart2_init(uint32_t u32Baud, uint32_t u32Parity);

/**
 * @brief Deinitialize UART2.
 */
void drv_uart2_deinit(void);

/**
 * @brief Send data through UART2.
 *
 * @param[in] pData   Data buffer.
 * @param[in] u16Size Data size.
 */
void drv_uart2_send(const uint8_t *pData, uint16_t u16Size);

/**
 * @brief Receive data from UART2.
 *
 * @param[out] pData     Receive buffer.
 * @param[in,out] u16Size Data size.
 *
 * @return true if data is received successfully, otherwise false.
 */
bool drv_uart2_receive(uint8_t *pData, uint16_t *u16Size);


//------------------------------------------------------------------------------
// UART3
//------------------------------------------------------------------------------

/**
 * @brief Initialize UART3.
 *
 * @param[in] u32Baud   Baud rate.
 * @param[in] u32Parity Parity configuration.
 */
void drv_uart3_init(uint32_t u32Baud, uint32_t u32Parity);

/**
 * @brief Deinitialize UART3.
 */
void drv_uart3_deinit(void);

/**
 * @brief Send data through UART3.
 *
 * @param[in] pData   Data buffer.
 * @param[in] u16Size Data size.
 */
void drv_uart3_send(const uint8_t *pData, uint16_t u16Size);

/**
 * @brief Receive data from UART3.
 *
 * @param[out] pData     Receive buffer.
 * @param[in,out] u16Size Data size.
 *
 * @return true if data is received successfully, otherwise false.
 */
bool drv_uart3_receive(uint8_t *pData, uint16_t *u16Size);