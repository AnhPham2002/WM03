#include "sv_protocol.h"
#include "drv_protocol.h"

static const Protocol_Access_Rule_t ProtocolAccessRuleTable[] = {
    /* CMD_ACCESS: 0x00 */
    {CMD_ACCESS, ACCESS_LV2, ACCESS_LEVEL_1},
    {CMD_ACCESS, ACCESS_LV3, ACCESS_LEVEL_1},
    {CMD_ACCESS, ACCESS_LV4, ACCESS_LEVEL_1},

    /* CMD_GET: 0x01 */
    {CMD_GET, CONFIG_MODULE_SERIAL, ACCESS_LEVEL_2},
    {CMD_GET, CONFIG_MCU_RESET_COUNT, ACCESS_LEVEL_4},
    {CMD_GET, CONFIG_TIME, ACCESS_LEVEL_2},
    {CMD_GET, CONFIG_IP_ENDPOINT, ACCESS_LEVEL_2},
    {CMD_GET, CONFIG_MODULE, ACCESS_LEVEL_2},
    {CMD_GET, CONFIG_PULSE_METER, ACCESS_LEVEL_2},
    {CMD_GET, CONFIG_MODBUS_METER, ACCESS_LEVEL_2},
    {CMD_GET, CONFIG_PRESSURE_SENSOR, ACCESS_LEVEL_2},

    /* CMD_SET: 0x02 */
    {CMD_SET, CONFIG_MODULE_SERIAL, ACCESS_LEVEL_4},
    {CMD_SET, CONFIG_TIME, ACCESS_LEVEL_3},
    {CMD_SET, CONFIG_IP_ENDPOINT, ACCESS_LEVEL_3},
    {CMD_SET, CONFIG_MODULE, ACCESS_LEVEL_3},
    {CMD_SET, CONFIG_PULSE_METER, ACCESS_LEVEL_3},
    {CMD_SET, CONFIG_MODBUS_METER, ACCESS_LEVEL_3},
    {CMD_SET, CONFIG_PRESSURE_SENSOR, ACCESS_LEVEL_3},
    {CMD_SET, CONFIG_REBOOT, ACCESS_LEVEL_4},
    {CMD_SET, CONFIG_RESET_SETTING, ACCESS_LEVEL_4},
    {CMD_SET, CONFIG_CHANGE_PASSWORD, ACCESS_LEVEL_4},
    {CMD_SET, CONFIG_RESET_PASSWORD, ACCESS_LEVEL_4},
    {CMD_SET, CONFIG_MCU_RESET_COUNT, ACCESS_LEVEL_4},
    {CMD_SET, CONFIG_ERASE_MEASUREMENT_DATA, ACCESS_LEVEL_4},
    {CMD_SET, CONFIG_ERASE_EVENT_DATA, ACCESS_LEVEL_4},
    {CMD_SET, CONFIG_ERASE_LOG_DATA, ACCESS_LEVEL_4},
    {CMD_SET, CONFIG_FACTORY_RESET, ACCESS_LEVEL_4},
    {CMD_SET, CONFIG_LATCH_IMMEDIATELY, ACCESS_LEVEL_4},
    {CMD_SET, CONFIG_PUSH_IMMEDIATELY, ACCESS_LEVEL_4},
    {CMD_SET, CONFIG_EVENT_CREATE, ACCESS_LEVEL_4},

    /* CMD_QUERY: 0x03 */
    {CMD_QUERY, QUERY_BOOTLOADER_VERSION, ACCESS_LEVEL_4},
    {CMD_QUERY, QUERY_FIRMWARE_VERSION, ACCESS_LEVEL_4},
    {CMD_QUERY, QUERY_POWER_SUPPLY_INFO, ACCESS_LEVEL_3},
    {CMD_QUERY, QUERY_SIM_NETWORK_INFO, ACCESS_LEVEL_3},
    {CMD_QUERY, QUERY_LATCH, ACCESS_LEVEL_3},
    {CMD_QUERY, QUERY_EVENT, ACCESS_LEVEL_3},
    {CMD_QUERY, QUERY_LOG, ACCESS_LEVEL_3},

    /* CMD_PUSH: 0x04 */
    {CMD_PUSH, PUSH_INFO, ACCESS_LEVEL_1},
    {CMD_PUSH, PUSH_LATCH, ACCESS_LEVEL_1},
    {CMD_PUSH, PUSH_EVENT, ACCESS_LEVEL_1},
    {CMD_PUSH, PUSH_MODULE_CONFIG, ACCESS_LEVEL_1},
    {CMD_PUSH, PUSH_PULSE_METER_CONFIG, ACCESS_LEVEL_1},
    {CMD_PUSH, PUSH_MODBUS_METER_CONFIG, ACCESS_LEVEL_1},
    {CMD_PUSH, PUSH_PRESSURE_SENSOR_CONFIG, ACCESS_LEVEL_1},

    /* CMD_OTA: 0x05 */
    {CMD_OTA, OTA_UPDATE_REQUEST, ACCESS_LEVEL_3},
    {CMD_OTA, OTA_SEND_INFO, ACCESS_LEVEL_3},
    {CMD_OTA, OTA_SEND_PACKET, ACCESS_LEVEL_3},
};

/*==================================================================================================
*                                PRIVATE FUNCTIONS DECLARATIONS
==================================================================================================*/

/**
 * @brief Check whether a command and ID combination is valid.
 *
 * Validates the ID against the list associated with the specified command.
 *
 * @param[in] u8Cmd Command code.
 * @param[in] u8Id  ID code.
 *
 * @return true if the command and ID combination is valid, otherwise false.
 */
static bool sv_protocol_check_valid_cmd(uint8_t u8Cmd, uint8_t u8Id);

/*==================================================================================================
*                                   PUBLIC FUNCTIONS DEFINITIONS
==================================================================================================*/

uint8_t sv_protocol_get_required_level(uint8_t u8Cmd, uint8_t u8Id)
{
    uint16_t u16Index;

    for (u16Index = 0; u16Index < ARRAY_SIZE(ProtocolAccessRuleTable); u16Index++)
    {
        if ((ProtocolAccessRuleTable[u16Index].u8Cmd == u8Cmd) && (ProtocolAccessRuleTable[u16Index].u8Id == u8Id))
        {
            return ProtocolAccessRuleTable[u16Index].u8RequiredLevel;
        }
    }

    return ACCESS_LEVEL_NONE;
}

bool sv_protocol_pack(bool bEncrypt, uint64_t u64Serial, uint8_t u8Cmd, uint8_t u8Id, const uint8_t *pPayload, uint16_t u16PayloadLen, uint8_t *pFrame, uint16_t *u16FrameLen)
{
    if (!sv_protocol_check_valid_cmd(u8Cmd, u8Id))
    {
        return false;
    }

    return drv_protocol_pack(bEncrypt, u64Serial, u8Cmd, u8Id, pPayload, u16PayloadLen, pFrame, u16FrameLen);
}

Protocol_Err_Code_t sv_protocol_unpack(const uint8_t *pFrame, uint16_t u16FrameLen, uint64_t *u64Serial, uint8_t *u8Cmd, uint8_t *u8Id, uint8_t *pPayload, uint16_t *u16PayloadLen)
{
    Protocol_Err_Code_t eErrCode = drv_protocol_unpack(pFrame, u16FrameLen, u64Serial, u8Cmd, u8Id, pPayload, u16PayloadLen);

    if ((eErrCode == PROTOCOL_ERR_SUCCESS) && (!sv_protocol_check_valid_cmd(*u8Cmd, *u8Id)))
    {
        return PROTOCOL_ERR_CMD_INVALID;
    }

    return eErrCode;
}

/*==================================================================================================
*                                   PRIVATE FUNCTIONS DEFINITIONS
==================================================================================================*/

static bool sv_protocol_check_valid_cmd(uint8_t u8Cmd, uint8_t u8Id)
{
    switch (u8Cmd)
    {
    case CMD_ACCESS:
        switch (u8Id)
        {
#define X(name, val) \
    case val:        \
        return true;
            PROTOCOL_ACCESS_ID_LIST(X)
#undef X
        }
        break;

    case CMD_GET:
    case CMD_SET:
        switch (u8Id)
        {
#define X(name, val) \
    case val:        \
        return true;
            PROTOCOL_CONFIG_ID_LIST(X)
#undef X
        }
        break;

    case CMD_QUERY:
        switch (u8Id)
        {
#define X(name, val) \
    case val:        \
        return true;
            PROTOCOL_QUERY_ID_LIST(X)
#undef X
        }
        break;

    case CMD_PUSH:
        switch (u8Id)
        {
#define X(name, val) \
    case val:        \
        return true;
            PROTOCOL_PUSH_ID_LIST(X)
#undef X
        }
        break;

    case CMD_OTA:
        switch (u8Id)
        {
#define X(name, val) \
    case val:        \
        return true;
            PROTOCOL_OTA_ID_LIST(X)
#undef X
        }
        break;

    default:
        break;
    }

    return false;
}
