#pragma once

#include "setting.h"
#include "drv_uart.h"

/**
 * @brief Initialize system debug.
 */
void sys_debug_init(void);

/**
 * @brief Send debug data.
 *
 * @param[in] pData   Data buffer.
 * @param[in] u16Size Data size.
 */
void sys_log(const uint8_t *pData, uint16_t u16Size);

/**
 * @brief Receive data from system console.
 *
 * @param[out] pData     Receive buffer.
 * @param[in,out] u16Size Data size.
 *
 * @return true if data is received successfully, otherwise false.
 */
bool sys_console(uint8_t *pData, uint16_t *u16Size);