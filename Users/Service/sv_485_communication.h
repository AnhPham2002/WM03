#pragma once

#include "drv_rs485.h"
#include "drv_wkup_pin.h"
#include "drv_led.h"

#define RS485_COMMUNICATION_BAUDRATE 19200
#define RS485_COMMUNICATION_SERIAL_CONFIG UART_PARITY_NONE
#define RS485_COMMUNICATION_TIMEOUT 60000

/**
 * @brief Initialize RS485 communication service.
 *
 * Configures the RS485 baud rate and serial communication parameters.
 */
void sv_485_communication_init(void);

/**
 * @brief Process RS485 power control.
 *
 * Turns on RS485 power when communication activity is detected and
 * turns it off after the communication timeout expires.
 */
void sv_485_communication_power_process(void);

/**
 * @brief Send data through RS485.
 *
 * @param[in] pData  Transmit data buffer.
 * @param[in] u16Size Data size in bytes.
 *
 * @return true if data is sent successfully, otherwise false.
 */
bool sv_485_communication_send(const uint8_t *pData, uint16_t u16Size);

/**
 * @brief Receive data through RS485.
 *
 * @param[out] pData  Receive data buffer.
 * @param[out] u16Size Received data size in bytes.
 *
 * @return true if data is received successfully, otherwise false.
 */
bool sv_485_communication_receive(uint8_t *pData, uint16_t *u16Size);