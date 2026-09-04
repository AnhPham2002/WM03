#pragma once

#include "drv_modbus.h"

#define MAX_MODBUS_METER_COUNT 4

/** @defgroup MODBUS_SerialConfig Modbus Serial Configuration
 *  @brief Modbus serial communication configuration.
 *  @{
 */
#define MODBUS_SERIAL_8N1  0x00
#define MODBUS_SERIAL_8O1  0x01
#define MODBUS_SERIAL_8E1  0x02
/** @} */

/** @defgroup MODBUS_Data_Type Modbus Data Type
 *  @brief Modbus data type definitions.
 *  @{
 */
#define MODBUS_INT16 0x01
#define MODBUS_UINT16 0x02
#define MODBUS_INT32 0x03
#define MODBUS_UINT32 0x04
#define MODBUS_FLOAT 0x05
#define MODBUS_INT64 0x06
#define MODBUS_UINT64 0x07
#define MODBUS_DOUBLE 0x08
/** @} */

/**
 * @brief  Modbus communication configuration.
 */
typedef struct __attribute__((packed))
{
    uint8_t u8SlaveAddress;
    uint32_t u32BaudRate;
    uint8_t u8SerialConfig;
    uint8_t u8ReadFuncCode;
} Modbus_Config_t;

/**
 * @brief  Configuration of a Modbus data parameter.
 */
typedef struct __attribute__((packed))
{
	uint8_t u8ParameterEnable;
    uint16_t u16RegisterAddress;
    uint8_t u8DataType;
    uint8_t u8WordSwap;
    int8_t s8Multiplier;
} Modbus_Parameter_t;

/**
 * @brief  Complete configuration of a Modbus meter.
 */
typedef struct __attribute__((packed))
{
    Modbus_Config_t sModbusConfig;
    Modbus_Parameter_t sForwardTotalizer;
    Modbus_Parameter_t sReverseTotalizer;
    Modbus_Parameter_t sFlowRate;
} Modbus_Meter_Config_t;

/**
 * @brief  Data read from a Modbus meter.
 */
typedef struct
{
    double dTotalForward;
    double dTotalReverse;
    double dFlowRate;
} Modbus_Meter_Data_t;

/**
 * @brief Set Modbus meter latch period flag.
 */
void sv_modbus_meter_set_latch_period(void);

/**
 * @brief Set Modbus meter configuration.
 *
 * @param[in] u8MeterIndex Meter index.
 * @param[in] pConfig      Modbus meter configuration.
 *
 * @return true if the configuration is set successfully, otherwise false.
 */
bool sv_modbus_meter_set_config(uint8_t u8MeterIndex, const Modbus_Meter_Config_t *pConfig);

/**
 * @brief Get Modbus meter data.
 *
 * Reads enabled parameters from the Modbus meter and decodes the received
 * data into totalizer and flow rate values.
 *
 * @param[in]  u8MeterIndex Meter index.
 * @param[out] pData        Modbus meter data.
 *
 * @return true if the meter data is read successfully, otherwise false.
 */
bool sv_modbus_meter_get_data(uint8_t u8MeterIndex, Modbus_Meter_Data_t *pData);