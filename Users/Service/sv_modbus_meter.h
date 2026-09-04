#pragma once

#include "drv_modbus.h"

#define MAX_MODBUS_METER_COUNT 4

/** @defgroup MODBUS_Parity Modbus Parity
 *  @brief Modbus parity configuration.
 *  @{
 */
#define MODBUS_PARITY_NONE 0x00
#define MODBUS_PARITY_ODD 0x01
#define MODBUS_PARITY_EVEN 0x02
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
    uint8_t u8Parity;
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
 * @brief Modbus meter processing step.
 */
typedef enum
{
	MODBUS_STEP_IDLE = 0,
	MODBUS_STEP_READING,
	MODBUS_STEP_OK
} Modbus_Meter_Step_t;

/**
 * @brief Set Modbus meter latch period flag.
 */
void sv_modbus_meter_set_latch_period(void);

/**
 * @brief Set Modbus meter configuration.
 *
 * @param[in] u8MeterCount Number of Modbus meters.
 * @param[in] pConfig      Meter configuration array.
 *
 * @return true if configuration is valid and set successfully, otherwise false.
 */
bool sv_modbus_meter_set_config(uint8_t u8MeterCount, const Modbus_Meter_Config_t *pConfig);

/**
 * @brief Process Modbus meter reading.
 *
 * @return true when all configured meters are processed, otherwise false.
 */
bool sv_modbus_meter_process(void);

/**
 * @brief Get Modbus meter data.
 *
 * @param[in]  u8MeterIndex Meter index.
 * @param[out] pData        Meter data.
 *
 * @return true if the meter index and data pointer are valid, otherwise false.
 */
bool sv_modbus_meter_get_data(uint8_t u8MeterIndex, Modbus_Meter_Data_t *pData);