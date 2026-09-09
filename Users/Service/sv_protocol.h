#pragma once

#include "drv_protocol.h"

#define PROTOCOL_CMD_LIST(X) \
    X(CMD_ACCESS, 0x00)      \
    X(CMD_GET, 0x01)         \
    X(CMD_SET, 0x02)         \
    X(CMD_QUERY, 0x03)       \
    X(CMD_PUSH, 0x04)        \
    X(CMD_OTA, 0x05)

typedef enum
{
#define X(name, val) name = val,
    PROTOCOL_CMD_LIST(X)
#undef X
} Protocol_Cmd_Code_t;

/* ID for CMD_ACCESS: 0x00 */
#define PROTOCOL_ACCESS_ID_LIST(X) \
    X(ACCESS_LV2, 0x00)            \
    X(ACCESS_LV3, 0x01)            \
    X(ACCESS_LV4, 0x02)

typedef enum
{
#define X(name, val) name = val,
    PROTOCOL_ACCESS_ID_LIST(X)
#undef X
} Protocol_Access_Id_Code_t;

#define PROTOCOL_ACCESS_TIMEOUT 60000 // ms

/* ID for CMD_SET and CMD_GET: 0x01 & 0x02 */
#define PROTOCOL_CONFIG_ID_LIST(X)         \
    X(CONFIG_MODULE_SERIAL, 0x00)          \
    X(CONFIG_TIME, 0x01)                   \
    X(CONFIG_IP_ENDPOINT, 0x02)            \
    X(CONFIG_MODULE, 0x03)                 \
    X(CONFIG_PULSE_METER, 0x04)            \
    X(CONFIG_MODBUS_METER, 0x05)           \
    X(CONFIG_PRESSURE_SENSOR, 0x06)        \
    X(CONFIG_REBOOT, 0x07)                 \
    X(CONFIG_RESET_SETTING, 0x08)          \
    X(CONFIG_CHANGE_PASSWORD, 0x09)        \
    X(CONFIG_RESET_PASSWORD, 0x10)         \
    X(CONFIG_MCU_RESET_COUNT, 0x11)        \
    X(CONFIG_ERASE_MEASUREMENT_DATA, 0x12) \
    X(CONFIG_ERASE_EVENT_DATA, 0x13)       \
    X(CONFIG_ERASE_LOG_DATA, 0x14)         \
    X(CONFIG_FACTORY_RESET, 0x15)          \
    X(CONFIG_LATCH_IMMEDIATELY, 0x16)      \
    X(CONFIG_PUSH_IMMEDIATELY, 0x17)       \
    X(CONFIG_EVENT_CREATE, 0x18)

typedef enum
{
#define X(name, val) name = val,
    PROTOCOL_CONFIG_ID_LIST(X)
#undef X
} Protocol_Config_Id_Code_t;

/* ID for CMD_QUERY: 0x03 */
#define PROTOCOL_QUERY_ID_LIST(X)     \
    X(QUERY_BOOTLOADER_VERSION, 0x00) \
    X(QUERY_FIRMWARE_VERSION, 0x01)   \
    X(QUERY_POWER_SUPPLY_INFO, 0x02)  \
    X(QUERY_SIM_NETWORK_INFO, 0x03)   \
    X(QUERY_LATCH, 0x04)              \
    X(QUERY_EVENT, 0x05)              \
    X(QUERY_LOG, 0x06)

typedef enum
{
#define X(name, val) name = val,
    PROTOCOL_QUERY_ID_LIST(X)
#undef X
} Protocol_Query_Id_Code_t;

/* ID for CMD_PUSH: 0x04 */
#define PROTOCOL_PUSH_ID_LIST(X)      \
    X(PUSH_INFO, 0x00)                \
    X(PUSH_LATCH, 0x01)               \
    X(PUSH_EVENT, 0x02)               \
    X(PUSH_MODULE_CONFIG, 0x03)       \
    X(PUSH_PULSE_METER_CONFIG, 0x04)  \
    X(PUSH_MODBUS_METER_CONFIG, 0x05) \
    X(PUSH_PRESSURE_SENSOR_CONFIG, 0x06)

typedef enum
{
#define X(name, val) name = val,
    PROTOCOL_PUSH_ID_LIST(X)
#undef X
} Protocol_Push_Id_Code_t;

/* ID for CMD_OTA: 0x05 */
#define PROTOCOL_OTA_ID_LIST(X) \
    X(OTA_UPDATE_REQUEST, 0x00) \
    X(OTA_SEND_INFO, 0x01)      \
    X(OTA_SEND_PACKET, 0x02)

typedef enum
{
#define X(name, val) name = val,
    PROTOCOL_OTA_ID_LIST(X)
#undef X
} Protocol_Ota_Id_Code_t;

#define OTA_MAX_PACKET_SIZE 1024

typedef enum
{
    ACCESS_LEVEL_1 = 1,
    ACCESS_LEVEL_2 = 2,
    ACCESS_LEVEL_3 = 3,
    ACCESS_LEVEL_4 = 4,
    ACCESS_LEVEL_NONE = 0xFF
} Protocol_Access_Level_t;

typedef struct
{
    uint8_t u8Cmd;
    uint8_t u8Id;
    uint8_t u8RequiredLevel;
} Protocol_Access_Rule_t;

/**
 * @brief Get the required access level for a command and ID.
 *
 * @param[in] u8Cmd Command code.
 * @param[in] u8Id  ID code.
 *
 * @return Required access level.
 */
uint8_t sv_protocol_get_required_level(uint8_t u8Cmd, uint8_t u8Id);

/**
 * @brief Pack data into a protocol frame.
 *
 * Validates the command and ID before packing the protocol frame.
 *
 * @param[in]     bEncrypt       Enable AES-128 encryption.
 * @param[in]     u64Serial      Module serial number.
 * @param[in]     u8Cmd          Command code.
 * @param[in]     u8Id           ID code.
 * @param[in]     pPayload       Payload data.
 * @param[in]     u16PayloadLen  Payload size in bytes.
 * @param[out]    pFrame         Output frame buffer.
 * @param[out]    u16FrameLen    Output frame size in bytes.
 *
 * @return true if the frame is packed successfully, otherwise false.
 */
bool sv_protocol_pack(bool bEncrypt, uint64_t u64Serial, uint8_t u8Cmd, uint8_t u8Id, const uint8_t *pPayload, uint16_t u16PayloadLen, uint8_t *pFrame, uint16_t *u16FrameLen);

/**
 * @brief Unpack and validate a protocol frame.
 *
 * Unpacks the protocol frame and validates the command and ID.
 *
 * @param[in]  pFrame         Input protocol frame.
 * @param[in]  u16FrameLen    Frame size in bytes.
 * @param[out] u64Serial      Module serial number.
 * @param[out] u8Cmd          Command code.
 * @param[out] u8Id           ID code.
 * @param[out] pPayload       Output payload buffer.
 * @param[out] u16PayloadLen  Output payload size in bytes.
 *
 * @return Protocol error code.
 */
Protocol_Err_Code_t sv_protocol_unpack(const uint8_t *pFrame, uint16_t u16FrameLen, uint64_t *u64Serial, uint8_t *u8Cmd, uint8_t *u8Id, uint8_t *pPayload, uint16_t *u16PayloadLen);