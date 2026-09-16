#pragma once

#include "sv_protocol.h"

#include "app_cellular.h"
#include "app_latch.h"

#define PROTOCOL_REQUEST_TIMEOUT 50

#define PROTOCOL_MAX_PACK_EVENT_COUNT 20

#define PROTOCOL_BUFFER_SIZE 2048

typedef enum
{
    PROTOCOL_DATA_SOURCE_CELLULAR = 0,
    PROTOCOL_DATA_SOURCE_RS485
} Protocol_Data_Source_t;

typedef struct
{
    uint8_t u8IdCode;
    uint8_t u8CfgCode;
    uint8_t u8Size;
    void *pData;
} Protocol_Config_Map_t;

#define CONFIG_MAP_SIZE 128

typedef enum
{
    CONFIG_MODULE_LATCH_PERIOD = 0,
    CONFIG_MODULE_PUSH_PERIOD,
    CONFIG_MODULE_TIMEZONE
} Config_Module_Id_t;

typedef enum
{
    CONFIG_PULSE_METER_ENABLE = 0,
    CONFIG_PULSE_METER_SERIAL_NUMBER,
    CONFIG_PULSE_METER_PULSE_FACTOR,
    CONFIG_PULSE_METER_PULSE_TYPE,
    CONFIG_PULSE_METER_PIN1,
    CONFIG_PULSE_METER_PIN2,
    CONFIG_PULSE_METER_EDGE_TYPE,
    CONFIG_PULSE_METER_DATA
} Config_Pulse_Meter_Id_t;

typedef enum
{
    CONFIG_MODBUS_METER_ENABLE = 0,
    CONFIG_MODBUS_METER_SERIAL_NUMBER,
    CONFIG_MODBUS_METER_SLAVE_ADDRESS,
    CONFIG_MODBUS_METER_BAUDRATE,
    CONFIG_MODBUS_METER_SERIAL_CONFIG,
    CONFIG_MODBUS_METER_READ_FUNC_CODE,
    CONFIG_MODBUS_METER_FORWARD_TOTAL_ENABLE,
    CONFIG_MODBUS_METER_FORWARD_TOTAL_REG_ADDR,
    CONFIG_MODBUS_METER_FORWARD_TOTAL_DATA_TYPE,
    CONFIG_MODBUS_METER_FORWARD_TOTAL_WORD_SWAP,
    CONFIG_MODBUS_METER_FORWARD_TOTAL_MULTIPLIER,
    CONFIG_MODBUS_METER_REVERSE_TOTAL_ENABLE,
    CONFIG_MODBUS_METER_REVERSE_TOTAL_REG_ADDR,
    CONFIG_MODBUS_METER_REVERSE_TOTAL_DATA_TYPE,
    CONFIG_MODBUS_METER_REVERSE_TOTAL_WORD_SWAP,
    CONFIG_MODBUS_METER_REVERSE_TOTAL_MULTIPLIER,
    CONFIG_MODBUS_METER_FLOW_RATE_ENABLE,
    CONFIG_MODBUS_METER_FLOW_RATE_TOTAL_REG_ADDR,
    CONFIG_MODBUS_METER_FLOW_RATE_TOTAL_DATA_TYPE,
    CONFIG_MODBUS_METER_FLOW_RATE_TOTAL_WORD_SWAP,
    CONFIG_MODBUS_METER_FLOW_RATE_TOTAL_MULTIPLIER
} Config_Modbus_Meter_Id_t;

typedef enum
{
    CONFIG_PRESSURE_SENSOR_ENABLE = 0,
    CONFIG_PRESSURE_SENSOR_SERIAL_NUMBER,
    CONFIG_PRESSURE_SENSOR_MIN_CURRENT,
    CONFIG_PRESSURE_SENSOR_MAX_CURRENT,
    CONFIG_PRESSURE_SENSOR_MIN_PRESSURE,
    CONFIG_PRESSURE_SENSOR_MAX_PRESSURE
} Config_Pressure_Sensor_Id_t;

/**
 * @brief Update protocol status and process pending requests.
 *
 * Checks the access timeout and processes pending reboot, setting reset,
 * and factory reset requests after the request timeout expires.
 */
void app_protocol_update(void);

/**
 * @brief Pack a latch data frame for transmission.
 *
 * Loads the specified latch data, packs it into a protocol payload,
 * and creates an encrypted PUSH frame.
 *
 * @param[in]  u16LatchIndex Latch index.
 * @param[out] pFrame        Output frame buffer.
 * @param[out] u16FrameLen   Output frame size in bytes.
 *
 * @return true if the frame is packed successfully, otherwise false.
 */
bool app_protocol_pack_push_latch(uint16_t u16LatchIndex, uint8_t *pFrame, uint16_t *u16FrameLen);

/**
 * @brief Pack event data into a protocol frame for transmission.
 *
 * Loads pending events starting from the specified index, packs them into
 * a protocol payload, and creates an encrypted PUSH frame.
 *
 * @param[in]  u16EventIndex    Starting event index.
 * @param[out] u8EventPackCount Number of events packed into the frame.
 * @param[out] pFrame           Output frame buffer.
 * @param[out] u16FrameLen      Output frame size in bytes.
 *
 * @return true if the frame is packed successfully, otherwise false.
 */
bool app_protcol_pack_push_event(uint16_t u16EventIndex, uint8_t *u8EventPackCount, uint8_t *pFrame, uint16_t *u16FrameLen);

/**
 * @brief Process a protocol frame.
 *
 * Unpacks and validates the received frame, processes the requested command,
 * and packs the response frame.
 *
 * @param[in]  eDataSource  Protocol data source.
 * @param[in]  pRxFrame     Received protocol frame.
 * @param[in]  u16RxFrameLen Received frame size in bytes.
 * @param[out] pTxFrame     Output response frame.
 * @param[out] u16TxFrameLen Output response frame size in bytes.
 *
 * @return Protocol error code.
 */
Protocol_Err_Code_t app_protocol_process(Protocol_Data_Source_t eDataSource, const uint8_t *pRxFrame, uint16_t u16RxFrameLen, uint8_t *pTxFrame, uint16_t *u16TxFrameLen);