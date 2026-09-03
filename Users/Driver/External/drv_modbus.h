#pragma once

#include "drv_rs485.h"

#define MODBUS_BUFFER_SIZE 256
#define MODBUS_RESPONSE_TIMEOUT 1000 // ms

typedef enum
{
    MODBUS_OK = 0,
    MODBUS_ERROR,
    MODBUS_NO_DATA,
    MODBUS_CRC_ERROR,
    MODBUS_EXCEPTION
} Modbus_Status_t;

typedef enum
{
	MODBUS_FUNC_READ_COILS = 0x01,
	MODBUS_FUNC_READ_DISCRETE_INPUT = 0x02,
	MODBUS_FUNC_READ_HOLDING_REGISTERS = 0x03,
	MODBUS_FUNC_READ_INPUT_REGISTERS = 0x04,
	MODBUS_FUNC_WRITE_SINGLE_COIL = 0x05,
	MODBUS_FUNC_WRITE_SINGLE_REGISTER = 0x06,
	MODBUS_FUNC_WRITE_MULTIPLE_COILS = 0x0F,
	MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS = 0x10
} Modbus_Function_Code_t;

typedef enum
{
	MODBUS_EX_ILLEGAL_FUNCTION		= 0x01,
	MODBUS_EX_ILLEGAL_DATA_ADDRESS  = 0x02,
	MODBUS_EX_ILLEGAL_DATA_VALUE    = 0x03,
	MODBUS_EX_SLAVE_DEVICE_FAILURE  = 0x04,
	MODBUS_EX_SLAVE_DEVICE_BUSY     = 0x06
} Modbus_Exception_Code_t;

/**
 * @brief Initialize Modbus communication.
 *
 * @param[in] u32Baud   Baud rate.
 * @param[in] u32Parity Parity configuration.
 */
void drv_modbus_init(uint32_t u32Baud, uint32_t u32Parity);

/**
 * @brief Read Modbus registers from a slave device.
 *
 * @param[in]  u8SlaveAddr    Slave address.
 * @param[in]  u8FuncCode     Read function code.
 * @param[in]  u16StartRegAddr Starting register address.
 * @param[in]  u16RegQuantity  Number of registers to read.
 * @param[out] pData           Data buffer.
 * @param[out] u16DataSize     Data size.
 *
 * @return Modbus status.
 */
Modbus_Status_t drv_modbus_master_read_register(uint8_t u8SlaveAddr, uint8_t u8FuncCode, uint16_t u16StartRegAddr, uint16_t u16RegQuantity, uint8_t *pData, uint16_t *u16DataSize);