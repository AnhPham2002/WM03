#pragma once

#include "drv_rtc.h"
#include "sv_eeprom.h"

#include "sv_flash.h"
#include "sv_time.h"

#include "sv_modbus_meter.h"
#include "sv_pressure_sensor.h"
#include "sv_pulse_meter.h"

#define STORAGE_BUFFER_SIZE 1024

#define VERSION_SIZE 10 // "Vxx.xx.xxx"

#define MODULE_SERIAL_SIZE 12
#define METER_SERIAL_SIZE 20

#define ALIGN4(x) (((x) + 3) & ~3)

typedef enum
{
    MODULE_TYPE = 0x00,
    PULSE_METER_TYPE = 0x01,
    MODBUS_METER_TYPE = 0x02,
    PRESSURE_SENSOR_TYPE = 0x03
} Meter_Type_t;

/*==================================================================================================
*                                      EEPROM HEADER
==================================================================================================*/

typedef struct __attribute__((packed))
{
    uint32_t u32Magic;
    uint8_t au8Version[10];
} Eeprom_Header_t;

/*==================================================================================================
*                             SEQUENCE SIZE FOR METADATA AND RUNTIME DATA
==================================================================================================*/
typedef uint64_t Eeprom_Sequence_t;

/*==================================================================================================
*                                      EEPROM METADATA
==================================================================================================*/

#define EEPROM_METADATA_AREA_COUNT 4

typedef struct __attribute__((packed))
{
    uint16_t u16NextLatchSaveIndex;
    uint16_t u16NextLatchLoadIndex;
    uint16_t u16LatchCount;
    uint16_t u16NextEventSaveIndex;
    uint16_t u16NextEventLoadIndex;
    uint16_t u16EventCount;
    uint16_t u16NextLogSaveIndex;
    uint16_t u16LogCount;
} Eeprom_Metadata_t;

typedef enum
{
    EEPROM_METADATA_AREA_1 = EEPROM_METADATA_ADDRESS,
    EEPROM_METADATA_AREA_2 = EEPROM_METADATA_AREA_1 + EEPROM_METADATA_SIZE / EEPROM_METADATA_AREA_COUNT,
    EEPROM_METADATA_AREA_3 = EEPROM_METADATA_AREA_2 + EEPROM_METADATA_SIZE / EEPROM_METADATA_AREA_COUNT,
    EEPROM_METADATA_AREA_4 = EEPROM_METADATA_AREA_3 + EEPROM_METADATA_SIZE / EEPROM_METADATA_AREA_COUNT,
} Eeprom_Metadata_Area_t;

/*==================================================================================================
*                                      EEPROM RUNTIME DATA
==================================================================================================*/

#define EEPROM_RUNTIME_DATA_AREA_COUNT 4

typedef struct __attribute__((packed))
{
    uint8_t u8ResetCount;
    Pulse_Count_t sPulseCount[MAX_PULSE_GATE_COUNT];
} Eeprom_Runtime_Data_t;

typedef enum
{
    EEPROM_RUNTIME_DATA_AREA_1 = EEPROM_RUNTIME_DATA_ADDRESS,
    EEPROM_RUNTIME_DATA_AREA_2 = EEPROM_RUNTIME_DATA_AREA_1 + EEPROM_RUNTIME_DATA_SIZE / EEPROM_RUNTIME_DATA_AREA_COUNT,
    EEPROM_RUNTIME_DATA_AREA_3 = EEPROM_RUNTIME_DATA_AREA_2 + EEPROM_RUNTIME_DATA_SIZE / EEPROM_RUNTIME_DATA_AREA_COUNT,
    EEPROM_RUNTIME_DATA_AREA_4 = EEPROM_RUNTIME_DATA_AREA_3 + EEPROM_RUNTIME_DATA_SIZE / EEPROM_RUNTIME_DATA_AREA_COUNT
} Eeprom_Runtime_Data_Area_t;

/*==================================================================================================
*                                      CONFIG PARAMETER
==================================================================================================*/

typedef struct __attribute__((packed))
{
    uint8_t au8Ipv4[15];
    uint8_t au8Port[5];
} Ip_Endpoint_t;

#define IPV4_ADDRESS "14.225.244.63"
#define IPV4_PORT "4399"

typedef struct __attribute__((packed))
{
    uint16_t u16LatchPeriod;
    uint16_t u16PushPeriod;
    float fTimezone;
} Module_Config_Parameter_t;

#define LATCH_PERIOD 15
#define PUSH_PERIOD 60
#define TIMEZONE (+7)

typedef struct __attribute__((packed))
{
    uint8_t u8Serial[METER_SERIAL_SIZE];
    Pulse_Meter_Config_t sConfig;
} Pulse_Meter_Config_Parameter_t;

#define PULSE_FACTOR 100
#define PULSE_PIN_1 PULSE_INPUT_1
#define PULSE_PIN_2 PULSE_INPUT_2
#define PULSE_EDGE_TYPE EDGE_FALLING
#define PULSE_TYPE PULSE_TYPE_SINGLE

typedef struct __attribute__((packed))
{
    uint8_t u8Serial[METER_SERIAL_SIZE];
    Modbus_Meter_Config_t sConfig;
} Modbus_Meter_Config_Parameter_t;

#define MODBUS_SLAVE_ADDRESS 1
#define MODBUS_BAUDRATE 19200
#define MODBUS_SERIAL MODBUS_SERIAL_8N1
#define MODBUS_READ_FUNCTION_CODE 0x03

#define FORWARD_TOTALIZER_ENABLE 1
#define FORWARD_TOTALIZER_ADDRESS 3017
#define FORWARD_TOTALIZER_DATA_TYPE MODBUS_UINT32
#define FORWARD_TOTALIZER_WORD_SWAP 0
#define FORWARD_TOTALIZER_MULTIPLIER 0

#define REVERSE_TOTALIZER_ENABLE 1
#define REVERSE_TOTALIZER_ADDRESS 3021
#define REVERSE_TOTALIZER_DATA_TYPE MODBUS_UINT32
#define REVERSE_TOTALIZER_WORD_SWAP 0
#define REVERSE_TOTALIZER_MULTIPLIER 0

#define FLOW_RATE_ENABLE 1
#define FLOW_RATE_ADDRESS 3002
#define FLOW_RATE_DATA_TYPE MODBUS_FLOAT
#define FLOW_RATE_WORD_SWAP 0
#define FLOW_RATE_MULTIPLIER 0

typedef struct __attribute__((packed))
{
    uint8_t u8Serial[METER_SERIAL_SIZE];
    Pressure_Sensor_Config_t sConfig;
} Pressure_Sensor_Config_Parameter_t;

#define SENSOR_MIN_CURRENT 4
#define SENSOR_MAX_CURRENT 20
#define SENSOR_MIN_PRESSURE 0
#define SENSOR_MAX_PRESSURE 6

/*==================================================================================================
*                                               EVENT
==================================================================================================*/

typedef struct __attribute__((packed))
{
    Date_Time_t sDateTime;
    uint8_t u8MeterType;
    uint8_t u8MeterIndex;
    uint8_t u8EventCode;
} Event_Data_t;

#define EVENT_PACKET_SIZE (ALIGN4(sizeof(Event_Data_t) + 2)) // +2 bytes CRC
#define MAX_EVENT_COUNT ((int)(EEPROM_EVENT_SIZE / EVENT_PACKET_SIZE))

typedef enum
{
    EVENT_RESET = 0x01,
    EVENT_MAGNETIC_DETEC = 0x02
} Event_Code_t;

/*==================================================================================================
*                                           LATCH RECORD
==================================================================================================*/

typedef struct __attribute__((packed))
{
    Date_Time_t sDateTime;
    Pulse_Meter_Data_t sPulseMeter[MAX_PULSE_METER_COUNT];
    Modbus_Meter_Data_t sModbusMeter[MAX_MODBUS_METER_COUNT];
    Pressure_Sensor_Data_t sPressureSensor[MAX_PRESSURE_SENSOR_COUNT];
} Latch_Data_t;

#define LATCH_PACKET_SIZE (ALIGN4(sizeof(Latch_Data_t) + 2)) // +2 bytes CRC
#define MAX_LATCH_COUNT ((int)(EEPROM_LATCH_SIZE / LATCH_PACKET_SIZE))

/*==================================================================================================
*                                PUBLIC FUNCTIONS DECLARATIONS
==================================================================================================*/

/**
 * @brief Initialize application storage.
 */
void app_storage_init(void);

/**
 * @brief Invalidate EEPROM header.
 *
 * Writes an invalid magic number to the EEPROM header.
 */
void app_storage_header_invalidate(void);

/**
 * @brief Set module serial number.
 *
 * @param[in] u64Serial Module serial number.
 *
 * @return true if the serial number is valid and saved successfully,
 *         otherwise false.
 */
bool app_storage_set_module_serial(uint64_t u64Serial);

/**
 * @brief Get module serial number.
 *
 * @return Module serial number.
 */
uint64_t app_storage_get_module_serial(void);

/**
 * @brief Get IP endpoint configuration.
 *
 * @param[out] pIpEndpoint IP endpoint configuration.
 */
void app_storage_get_ip_endpoint(Ip_Endpoint_t *pIpEndpoint);

/**
 * @brief Get latch period.
 *
 * @return Latch period.
 */
uint16_t app_storage_get_latch_period(void);

/**
 * @brief Get record period.
 *
 * @return Record period.
 */
uint16_t app_storage_get_record_period(void);

/**
 * @brief Get timezone offset.
 *
 * @return Timezone offset.
 */
float app_storage_get_timezone(void);

/**
 * @brief Save event data to storage.
 *
 * @param[in] pEvent Event data.
 *
 * @return true if the event is saved successfully, otherwise false.
 */
bool app_storage_event_save(const Event_Data_t *pEvent);

/**
 * @brief Load the next event data from storage.
 *
 * @param[in]  u16EventIndex Event index.
 * @param[out] pEvent        Event data.
 *
 * @return true if the event is loaded successfully, otherwise false.
 */
bool app_storage_event_load_next(uint16_t u16EventIndex, Event_Data_t *pEvent);

/**
 * @brief Load the latest event data from storage.
 *
 * @param[in]  u16EventIndex Event index.
 * @param[out] pEvent        Event data.
 *
 * @return true if the event is loaded successfully, otherwise false.
 */
bool app_storage_event_load_latest(uint16_t u16EventIndex, Event_Data_t *pEvent);

/**
 * @brief Update event load index.
 *
 * @param[in] u16EventCount Number of stored events.
 */
void app_storage_event_update_load_index(uint16_t u16EventCount);

/**
 * @brief Clear all stored events.
 */
void app_storage_event_clear(void);

/**
 * @brief Save latch data to storage.
 *
 * @param[in] pLatch Latch data.
 *
 * @return true if the latch data is saved successfully, otherwise false.
 */
bool app_storage_latch_save(const Latch_Data_t *pLatch);

/**
 * @brief Load the next latch data from storage.
 *
 * @param[in]  u16LatchIndex Latch index.
 * @param[out] pLatch        Latch data.
 *
 * @return true if the latch data is loaded successfully, otherwise false.
 */
bool app_storage_latch_load_next(uint16_t u16LatchIndex, Latch_Data_t *pLatch);

/**
 * @brief Load the latest latch data from storage.
 *
 * @param[in]  u16LatchIndex Latch index.
 * @param[out] pLatch        Latch data.
 *
 * @return true if the latch data is loaded successfully, otherwise false.
 */
bool app_storage_latch_load_latest(uint16_t u16LatchIndex, Latch_Data_t *pLatch);

/**
 * @brief Update latch load index.
 *
 * @param[in] u16LatchCount Number of stored latches.
 */
void app_storage_latch_update_load_index(uint16_t u16LatchCount);

/**
 * @brief Clear all stored latches.
 */
void app_storage_latch_clear(void);

/**
 * @brief Update pulse count in runtime data.
 *
 * Verifies the pulse count CRC before updating and saving the runtime data.
 *
 * @return true if the pulse count is valid and updated successfully,
 *         otherwise false.
 */
bool app_storage_runtime_data_update(void);

/**
 * @brief Increment the reset count.
 */
void app_storage_increase_reset_count(void);

/**
 * @brief Set the reset count.
 *
 * @param[in] u8Count Reset count value.
 */
void app_storage_set_reset_count(uint8_t u8Count);