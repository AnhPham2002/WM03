#include "app_protocol.h"

static uint32_t u32LastTimeAccess;
static Protocol_Access_Level_t eProtocolCurrentLevel = ACCESS_LEVEL_1;

static uint32_t u32TimeRequest;

static bool bRebbotFlag = false;
static bool bSettingResetRequestFlag = false;
static bool bFactoryResetRequestFlag = false;

static uint8_t au8ProtocolBuf[PROTOCOL_BUFFER_SIZE];
static uint16_t u16ProtocolBufLen;

/*==================================================================================================
*                                PRIVATE FUNCTIONS DECLARATIONS
==================================================================================================*/

/**
 * @brief Check access permission for a protocol command.
 *
 * Allows cellular data sources to bypass access level verification.
 * For other sources, checks whether the current access level meets
 * the required access level of the command and ID.
 *
 * @param[in] u8Cmd         Command code.
 * @param[in] u8Id          ID code.
 * @param[in] eCurrentLevel Current access level.
 * @param[in] eSource       Protocol data source.
 *
 * @return true if access is permitted, otherwise false.
 */
static bool app_protocol_check_access(uint8_t u8Cmd, uint8_t u8Id, Protocol_Access_Level_t eCurrentLevel, Protocol_Data_Source_t eSource);

/**
 * @brief Pack an acknowledgment frame.
 *
 * Includes the current date and time and the protocol error code
 * in the acknowledgment payload.
 *
 * @param[in]  u8Cmd         Command code.
 * @param[in]  u8Id          ID code.
 * @param[in]  eErrCode      Protocol error code.
 * @param[out] pFrame        Output frame buffer.
 * @param[out] u16FrameLen   Output frame size in bytes.
 *
 * @return true if the acknowledgment frame is packed successfully,
 *         otherwise false.
 */
static bool app_protocol_pack_ack(uint8_t u8Cmd, uint8_t u8Id, Protocol_Err_Code_t eErrCode, uint8_t *pFrame, uint16_t *u16FrameLen);

/**
 * @brief Pack latch data into a protocol payload.
 *
 * Packs the timestamp, meter serial numbers and corresponding measurement
 * data into the protocol payload.
 *
 * @param[in]  pLatch        Latch data.
 * @param[out] pPayload      Output payload buffer.
 * @param[out] u16PayloadLen Output payload size in bytes.
 */
static void app_protocol_pack_latch_payload(const Latch_Data_t *pLatch, uint8_t *pPayload, uint16_t *u16PayloadLen);

/**
 * @brief Pack event data into a protocol payload.
 *
 * Packs the current timestamp, event timestamps, meter serial numbers,
 * and event codes into the protocol payload.
 *
 * @param[in]  pEventList    Event data list.
 * @param[in]  u8EventCount  Number of events.
 * @param[out] pPayload      Output payload buffer.
 * @param[out] u16PayloadLen Output payload size in bytes.
 */
static void app_protocol_pack_event_payload(const Event_Data_t *pEventList, uint8_t u8EventCount, uint8_t *pPayload, uint16_t *u16PayloadLen);

/**
 * @brief Handle access level authentication.
 *
 * Verifies the received password and updates the current access level
 * when authentication is successful.
 *
 * @param[in]  u8Id            Access level ID.
 * @param[in]  pRxPayload      Received password data.
 * @param[in]  u16RxPayloadLen Received payload size in bytes.
 * @param[out] pTxFrame        Output acknowledgment frame.
 * @param[out] u16TxFrameLen   Output frame size in bytes.
 *
 * @return Protocol error code.
 */
static Protocol_Err_Code_t app_protocol_access_handler(uint8_t u8Id, const uint8_t *pRxPayload, uint16_t u16RxPayloadLen, uint8_t *pTxFrame, uint16_t *u16TxFrameLen);

/**
 * @brief Handle GET configuration request.
 *
 * Reads the requested configuration parameters and packs them into
 * an encrypted protocol response frame.
 *
 * @param[in]  u8Id            Configuration ID.
 * @param[in]  pRxPayload      Received request payload.
 * @param[in]  u16RxPayloadLen Received payload size in bytes.
 * @param[out] pTxFrame        Output response frame.
 * @param[out] u16TxFrameLen   Output response frame size in bytes.
 *
 * @return Protocol error code.
 */
static Protocol_Err_Code_t app_protocol_get_handler(uint8_t u8Id, const uint8_t *pRxPayload, uint16_t u16RxPayloadLen, uint8_t *pTxFrame, uint16_t *u16TxFrameLen);

/**
 * @brief Handle SET configuration request.
 *
 * Processes the requested configuration parameter and saves the updated
 * configuration to EEPROM.
 *
 * @param[in]  u8Id            Configuration ID.
 * @param[in]  pRxPayload      Received request payload.
 * @param[in]  u16RxPayloadLen Received payload size in bytes.
 * @param[out] pTxFrame        Output acknowledgment frame.
 * @param[out] u16TxFrameLen   Output frame size in bytes.
 *
 * @return Protocol error code.
 */
static Protocol_Err_Code_t app_protocol_set_handler(uint8_t u8Id, const uint8_t *pRxPayload, uint16_t u16RxPayloadLen, uint8_t *pTxFrame, uint16_t *u16TxFrameLen);

/**
 * @brief Handle QUERY request.
 *
 * Processes the requested query and packs the corresponding response
 * into an encrypted protocol frame.
 *
 * @param[in]  u8Id            Query ID.
 * @param[in]  pRxPayload      Received request payload.
 * @param[in]  u16RxPayloadLen Received payload size in bytes.
 * @param[out] pTxFrame        Output response frame.
 * @param[out] u16TxFrameLen   Output response frame size in bytes.
 *
 * @return Protocol error code.
 */
static Protocol_Err_Code_t app_protocol_query_handler(uint8_t u8Id, const uint8_t *pRxPayload, uint16_t u16RxPayloadLen, uint8_t *pTxFrame, uint16_t *u16TxFrameLen);
// static Protocol_Err_Code_t app_protocol_ota_handler(uint8_t u8Id, uint8_t *pRxPayload, uint16_t u16RxPayloadLen, uint8_t *pTxFrame, uint16_t *u16TxFrameLen);

/*==================================================================================================
*                                   PUBLIC FUNCTIONS DEFINITIONS
==================================================================================================*/

void app_protocol_update(void)
{
    if (sys_time_ms() - u32LastTimeAccess >= PROTOCOL_ACCESS_TIMEOUT)
    {
        eProtocolCurrentLevel = ACCESS_LEVEL_1;
    }

    if (bRebbotFlag && (sys_time_ms() - u32TimeRequest > PROTOCOL_REQUEST_TIMEOUT))
    {
        sys_reset();
    }

    if (bSettingResetRequestFlag && (sys_time_ms() - u32TimeRequest > PROTOCOL_REQUEST_TIMEOUT))
    {
        app_storage_config_parameter_default();
        sys_reset();
    }

    if (bFactoryResetRequestFlag && (sys_time_ms() - u32TimeRequest > PROTOCOL_REQUEST_TIMEOUT))
    {
        app_storage_header_invalidate();
        sys_reset();
    }
}

bool app_protocol_pack_push_latch(uint16_t u16LatchIndex, uint8_t *pFrame, uint16_t *u16FrameLen)
{
    Latch_Data_t sLatchData;

    if (app_storage_latch_load_next(u16LatchIndex, &sLatchData))
    {
        app_protocol_pack_latch_payload(&sLatchData, au8ProtocolBuf, &u16ProtocolBufLen);
        return sv_protocol_pack(true, app_storage_get_module_serial(), CMD_PUSH, PUSH_LATCH, au8ProtocolBuf, u16ProtocolBufLen, pFrame, u16FrameLen);
    }

    return false;
}

bool app_protcol_pack_push_event(uint16_t u16EventIndex, uint8_t *u8EventPackCount, uint8_t *pFrame, uint16_t *u16FrameLen)
{
    Event_Data_t sEventList[PROTOCOL_MAX_PACK_EVENT_COUNT];
    *u8EventPackCount = 0;

    while (*u8EventPackCount < PROTOCOL_MAX_PACK_EVENT_COUNT)
    {
        if (app_storage_event_load_next(u16EventIndex + (*u8EventPackCount), &sEventList[*u8EventPackCount]))
        {
            (*u8EventPackCount)++;
        }
        else
        {
            break;
        }
    }

    if (*u8EventPackCount == 0)
    {
        *u16FrameLen = 0;
        return false;
    }

    app_protocol_pack_event_payload(sEventList, *u8EventPackCount, au8ProtocolBuf, &u16ProtocolBufLen);
    return sv_protocol_pack(true, app_storage_get_module_serial(), CMD_PUSH, PUSH_EVENT, au8ProtocolBuf, u16ProtocolBufLen, pFrame, u16FrameLen);
}

Protocol_Err_Code_t app_protocol_process(Protocol_Data_Source_t eDataSource, const uint8_t *pRxFrame, uint16_t u16RxFrameLen, uint8_t *pTxFrame, uint16_t *u16TxFrameLen)
{
    uint64_t u64ModuleSerial;
    uint8_t u8Cmd;
    uint8_t u8Id;
    uint8_t *pRxPayload = au8ProtocolBuf;
    uint16_t u16RxPayloadLen;
    Protocol_Err_Code_t eErrCode;

    u32LastTimeAccess = sys_time_ms();

    eErrCode = sv_protocol_unpack(pRxFrame, u16RxFrameLen, &u64ModuleSerial, &u8Cmd, &u8Id, pRxPayload, &u16RxPayloadLen);
    if (eErrCode == PROTOCOL_ERR_NULL_POINTER)
    {
        return PROTOCOL_ERR_NULL_POINTER;
    }

    if ((u64ModuleSerial != app_storage_get_module_serial()) && (u64ModuleSerial != PROTOCOL_MODULE_SERIAL_COMMON))
    {
        return PROTOCOL_ERR_MODULE_SERIAL_INVALID;
    }

    if ((eErrCode == PROTOCOL_ERR_CMD_INVALID) || (eErrCode == PROTOCOL_ERR_CRC16_INVALID) || (eErrCode == PROTOCOL_ERR_FRAME_INVALID))
    {
        app_protocol_pack_ack(u8Cmd, u8Id, eErrCode, pTxFrame, u16TxFrameLen);
        return eErrCode;
    }

    if (!app_protocol_check_access(u8Cmd, u8Id, eProtocolCurrentLevel, eDataSource))
    {
        app_protocol_pack_ack(u8Cmd, u8Id, PROTOCOL_ERR_ACCESS_DENIED, pTxFrame, u16TxFrameLen);
        return PROTOCOL_ERR_ACCESS_DENIED;
    }

    switch (u8Cmd)
    {
    case CMD_ACCESS:
        app_protocol_access_handler(u8Id, pRxPayload, u16RxPayloadLen, pTxFrame, u16TxFrameLen);
        break;

    case CMD_GET:
        app_protocol_get_handler(u8Id, pRxPayload, u16RxPayloadLen, pTxFrame, u16TxFrameLen);
        sys_log(pTxFrame, u16RxFrameLen);
        break;

    case CMD_SET:
        app_protocol_set_handler(u8Id, pRxPayload, u16RxPayloadLen, pTxFrame, u16TxFrameLen);
        break;

    case CMD_QUERY:
        app_protocol_query_handler(u8Id, pRxPayload, u16RxPayloadLen, pTxFrame, u16TxFrameLen);
        break;

        // case CMD_PUSH:
        //     break;

        // case CMD_OTA:
        //     app_protocol_ota_handler(u8Id, pRxPayload, u16RxPayloadLen, pTxFrame, u16TxFrameLen);
        //     break;

    default:
        app_protocol_pack_ack(u8Cmd, u8Id, PROTOCOL_ERR_CMD_INVALID, pTxFrame, u16TxFrameLen);
        return PROTOCOL_ERR_CMD_INVALID;
    }

    return PROTOCOL_ERR_SUCCESS;
}

/*==================================================================================================
*                                   PRIVATE FUNCTIONS DEFINITIONS
==================================================================================================*/

static bool app_protocol_check_access(uint8_t u8Cmd, uint8_t u8Id, Protocol_Access_Level_t eCurrentLevel, Protocol_Data_Source_t eSource)
{
    if (eSource == PROTOCOL_DATA_SOURCE_CELLULAR)
    {
        return true;
    }

    uint8_t u8RequiredLevel = sv_protocol_get_required_level(u8Cmd, u8Id);

    if (u8RequiredLevel == ACCESS_LEVEL_NONE)
    {
        return false;
    }

    if ((uint8_t)eCurrentLevel < u8RequiredLevel)
    {
        return false;
    }

    return true;
}

static bool app_protocol_pack_ack(uint8_t u8Cmd, uint8_t u8Id, Protocol_Err_Code_t eErrCode, uint8_t *pFrame, uint16_t *u16FrameLen)
{
    uint8_t au8Payload[16];
    uint8_t *p = au8Payload;

    Date_Time_t sDateTimeSend;
    sv_time_get_date_time(&sDateTimeSend);
    memcpy(p, &sDateTimeSend, sizeof(sDateTimeSend));
    p += sizeof(sDateTimeSend);
    *p++ = (uint8_t)eErrCode;

    return sv_protocol_pack(false, app_storage_get_module_serial(), u8Cmd, u8Id, au8Payload, p - au8Payload, pFrame, u16FrameLen);
}

static void app_protocol_pack_latch_payload(const Latch_Data_t *pLatch, uint8_t *pPayload, uint16_t *u16PayloadLen)
{
    uint8_t *p = au8ProtocolBuf;
    uint8_t au8MeterSerial[METER_SERIAL_SIZE];
    uint8_t u8MeterSerialLen;
    Date_Time_t sDateTimeSend;

    sv_time_get_date_time(&sDateTimeSend);
    memcpy(p, &sDateTimeSend, sizeof(sDateTimeSend));
    p += sizeof(sDateTimeSend);

    memcpy(p, &pLatch->sDateTime, sizeof(pLatch->sDateTime));
    p += sizeof(pLatch->sDateTime);

    for (uint8_t i = 0; i < MAX_PULSE_METER_COUNT; i++)
    {
        u8MeterSerialLen = app_storage_get_meter_serial(PULSE_METER_TYPE, i, au8MeterSerial);
        if (u8MeterSerialLen == 0)
        {
            continue;
        }

        *p++ = (uint8_t)PULSE_METER_TYPE;
        *p++ = u8MeterSerialLen;
        memcpy(p, au8MeterSerial, u8MeterSerialLen);
        p += u8MeterSerialLen;

        memcpy(p, &pLatch->sPulseMeter[i].dTotalForward, sizeof(pLatch->sPulseMeter[i].dTotalForward));
        p += sizeof(pLatch->sPulseMeter[i].dTotalForward);

        memcpy(p, &pLatch->sPulseMeter[i].dTotalReverse, sizeof(pLatch->sPulseMeter[i].dTotalReverse));
        p += sizeof(pLatch->sPulseMeter[i].dTotalReverse);

        memcpy(p, &pLatch->sPulseMeter[i].dFlowRate, sizeof(pLatch->sPulseMeter[i].dFlowRate));
        p += sizeof(pLatch->sPulseMeter[i].dFlowRate);
    }

    for (uint8_t i = 0; i < MAX_MODBUS_METER_COUNT; i++)
    {
        u8MeterSerialLen = app_storage_get_meter_serial(MODBUS_METER_TYPE, i, au8MeterSerial);
        if (u8MeterSerialLen == 0)
        {
            continue;
        }

        *p++ = (uint8_t)MODBUS_METER_TYPE;
        *p++ = u8MeterSerialLen;
        memcpy(p, au8MeterSerial, u8MeterSerialLen);
        p += u8MeterSerialLen;

        memcpy(p, &pLatch->sModbusMeter[i].dTotalForward, sizeof(pLatch->sModbusMeter[i].dTotalForward));
        p += sizeof(pLatch->sModbusMeter[i].dTotalForward);

        memcpy(p, &pLatch->sModbusMeter[i].dTotalReverse, sizeof(pLatch->sModbusMeter[i].dTotalReverse));
        p += sizeof(pLatch->sModbusMeter[i].dTotalReverse);

        memcpy(p, &pLatch->sModbusMeter[i].dFlowRate, sizeof(pLatch->sModbusMeter[i].dFlowRate));
        p += sizeof(pLatch->sModbusMeter[i].dFlowRate);
    }

    for (uint8_t i = 0; i < MAX_PRESSURE_SENSOR_COUNT; i++)
    {
        u8MeterSerialLen = app_storage_get_meter_serial(PRESSURE_SENSOR_TYPE, i, au8MeterSerial);
        if (u8MeterSerialLen == 0)
        {
            continue;
        }

        *p++ = (uint8_t)PRESSURE_SENSOR_TYPE;
        *p++ = u8MeterSerialLen;
        memcpy(p, au8MeterSerial, u8MeterSerialLen);
        p += u8MeterSerialLen;

        memcpy(p, &pLatch->sPressureSensor[i].fPressure, sizeof(pLatch->sPressureSensor[i].fPressure));
        p += sizeof(pLatch->sPressureSensor[i].fPressure);
    }

    *u16PayloadLen = p - au8ProtocolBuf;
    memmove(pPayload, au8ProtocolBuf, *u16PayloadLen);
}

static void app_protocol_pack_event_payload(const Event_Data_t *pEventList, uint8_t u8EventCount, uint8_t *pPayload, uint16_t *u16PayloadLen)
{
    uint8_t *p = au8ProtocolBuf;
    uint8_t au8MeterSerial[METER_SERIAL_SIZE];
    uint8_t u8MeterSerialLen;
    Date_Time_t sDateTimeSend;

    sv_time_get_date_time(&sDateTimeSend);
    memcpy(p, &sDateTimeSend, sizeof(sDateTimeSend));
    p += sizeof(sDateTimeSend);

    if (u8EventCount > PROTOCOL_MAX_PACK_EVENT_COUNT)
    {
        u8EventCount = PROTOCOL_MAX_PACK_EVENT_COUNT;
    }

    for (uint8_t i = 0; i < u8EventCount; i++)
    {

        memcpy(p, &pEventList[i].sDateTime, sizeof(pEventList[i].sDateTime));
        p += sizeof(pEventList[i].sDateTime);

        u8MeterSerialLen = app_storage_get_meter_serial(pEventList[i].u8MeterType, pEventList[i].u8MeterIndex, au8MeterSerial);
        *p++ = u8MeterSerialLen;
        memcpy(p, au8MeterSerial, u8MeterSerialLen);
        p += u8MeterSerialLen;

        *p++ = pEventList[i].u8EventCode;
    }

    *u16PayloadLen = p - au8ProtocolBuf;
    memmove(pPayload, au8ProtocolBuf, *u16PayloadLen);
}

static Protocol_Err_Code_t app_protocol_access_handler(uint8_t u8Id, const uint8_t *pRxPayload, uint16_t u16RxPayloadLen, uint8_t *pTxFrame, uint16_t *u16TxFrameLen)
{
    bool bPasswordCorrect = false;
    static uint8_t au8Password[PASSWORD_LENGTH];

    if (u16RxPayloadLen != PASSWORD_LENGTH)
    {
        app_protocol_pack_ack(CMD_ACCESS, u8Id, PROTOCOL_ERR_PASSWORD_INCORRECT, pTxFrame, u16TxFrameLen);
        return PROTOCOL_ERR_PASSWORD_INCORRECT;
    }

    switch (u8Id)
    {
    case ACCESS_LV2:
        app_storage_get_password(ACCESS_LV2, au8Password);
        if (memcmp(pRxPayload, au8Password, PASSWORD_LENGTH) == 0)
        {
            eProtocolCurrentLevel = ACCESS_LEVEL_2;
            bPasswordCorrect = true;
        }
        break;

    case ACCESS_LV3:
        app_storage_get_password(ACCESS_LV3, au8Password);
        if (memcmp(pRxPayload, au8Password, PASSWORD_LENGTH) == 0)
        {
            eProtocolCurrentLevel = ACCESS_LEVEL_3;
            bPasswordCorrect = true;
        }
        break;

    case ACCESS_LV4:
        app_storage_get_password(ACCESS_LV4, au8Password);
        if (memcmp(pRxPayload, au8Password, PASSWORD_LENGTH) == 0)
        {
            eProtocolCurrentLevel = ACCESS_LEVEL_4;
            bPasswordCorrect = true;
        }
        break;

    default:
        app_protocol_pack_ack(CMD_ACCESS, u8Id, PROTOCOL_ERR_CMD_INVALID, pTxFrame, u16TxFrameLen);
        return PROTOCOL_ERR_CMD_INVALID;
    }

    if (bPasswordCorrect)
    {
        app_protocol_pack_ack(CMD_ACCESS, u8Id, PROTOCOL_ERR_SUCCESS, pTxFrame, u16TxFrameLen);
        u32LastTimeAccess = sys_time_ms();
        return PROTOCOL_ERR_SUCCESS;
    }
    else
    {
        app_protocol_pack_ack(CMD_ACCESS, u8Id, PROTOCOL_ERR_PASSWORD_INCORRECT, pTxFrame, u16TxFrameLen);
        return PROTOCOL_ERR_PASSWORD_INCORRECT;
    }
}

static Protocol_Err_Code_t app_protocol_get_handler(uint8_t u8Id, const uint8_t *pRxPayload, uint16_t u16RxPayloadLen, uint8_t *pTxFrame, uint16_t *u16TxFrameLen)
{
    bool bFailFlag = false;
    uint8_t au8TxPayload[512];
    uint8_t *p = au8TxPayload;

    switch (u8Id)
    {
    case CONFIG_MODULE_SERIAL:
    {
        uint64_t u64ModuleSerial = app_storage_get_module_serial();
        memcpy(p, &u64ModuleSerial, sizeof(u64ModuleSerial));
        p += sizeof(u64ModuleSerial);
        break;
    }

    case CONFIG_TIME:
    {
        Date_Time_t sDateTime;
        sv_time_get_date_time(&sDateTime);
        memcpy(p, &sDateTime, sizeof(sDateTime));
        p += sizeof(sDateTime);
        break;
    }

    case CONFIG_IP_ENDPOINT:
    {
        Ip_Endpoint_t sIpEndpoint;
        app_storage_get_ip_endpoint(&sIpEndpoint);
        memcpy(p, &sIpEndpoint, sizeof(sIpEndpoint));
        p += sizeof(sIpEndpoint);
        break;
    }

    case CONFIG_MODULE:
    {
        Module_Config_Parameter_t sModuleConfig;

        if (u16RxPayloadLen == 0)
        {
            bFailFlag = true;
            break;
        }

        app_storage_get_module_config(&sModuleConfig);

        for (uint16_t i = 0; i < u16RxPayloadLen; i++)
        {
            *p++ = pRxPayload[i];

            switch (pRxPayload[i])
            {
            case CONFIG_MODULE_LATCH_PERIOD:
                memcpy(p, &sModuleConfig.u16LatchPeriod, sizeof(sModuleConfig.u16LatchPeriod));
                p += sizeof(sModuleConfig.u16LatchPeriod);
                break;

            case CONFIG_MODULE_PUSH_PERIOD:
                memcpy(p, &sModuleConfig.u16PushPeriod, sizeof(sModuleConfig.u16PushPeriod));
                p += sizeof(sModuleConfig.u16PushPeriod);
                break;

            case CONFIG_MODULE_TIMEZONE:
                memcpy(p, &sModuleConfig.fTimezone, sizeof(sModuleConfig.fTimezone));
                p += sizeof(sModuleConfig.fTimezone);
                break;

            default:
                bFailFlag = true;
                break;
            }
        }

        break;
    }

    case CONFIG_PULSE_METER:
    {
        Pulse_Meter_Config_Parameter_t sPulseMeterConfig;

        if (u16RxPayloadLen <= 1)
        {
            bFailFlag = true;
            break;
        }

        if (!app_storage_get_pulse_meter_config(pRxPayload[0], &sPulseMeterConfig))
        {
            bFailFlag = true;
            break;
        }

        for (uint16_t i = 1; i < u16RxPayloadLen; i++)
        {
            *p++ = pRxPayload[i];

            switch (pRxPayload[i])
            {
            case CONFIG_PULSE_METER_SERIAL_NUMBER:
                memcpy(p, &sPulseMeterConfig.u8Serial, METER_SERIAL_SIZE);
                p += METER_SERIAL_SIZE;
                break;

            case CONFIG_PULSE_METER_PULSE_FACTOR:
                memcpy(p, &sPulseMeterConfig.sConfig.u16PulseFactor, sizeof(sPulseMeterConfig.sConfig.u16PulseFactor));
                p += sizeof(sPulseMeterConfig.sConfig.u16PulseFactor);
                break;

            case CONFIG_PULSE_METER_PIN1:
                memcpy(p, &sPulseMeterConfig.sConfig.sPulseConfig.u8Pin1Select, sizeof(sPulseMeterConfig.sConfig.sPulseConfig.u8Pin1Select));
                p += sizeof(sPulseMeterConfig.sConfig.sPulseConfig.u8Pin1Select);
                break;

            case CONFIG_PULSE_METER_PIN2:
                memcpy(p, &sPulseMeterConfig.sConfig.sPulseConfig.u8Pin2Select, sizeof(sPulseMeterConfig.sConfig.sPulseConfig.u8Pin2Select));
                p += sizeof(sPulseMeterConfig.sConfig.sPulseConfig.u8Pin2Select);
                break;

            case CONFIG_PULSE_METER_PULSE_TYPE:
                memcpy(p, &sPulseMeterConfig.sConfig.sPulseConfig.u8PulseType, sizeof(sPulseMeterConfig.sConfig.sPulseConfig.u8PulseType));
                p += sizeof(sPulseMeterConfig.sConfig.sPulseConfig.u8PulseType);
                break;

            case CONFIG_PULSE_METER_EDGE_TYPE:
                memcpy(p, &sPulseMeterConfig.sConfig.sPulseConfig.u8EdgeType, sizeof(sPulseMeterConfig.sConfig.sPulseConfig.u8EdgeType));
                p += sizeof(sPulseMeterConfig.sConfig.sPulseConfig.u8EdgeType);
                break;

            default:
                bFailFlag = true;
                break;
            }
        }

        break;
    }

    case CONFIG_MODBUS_METER:
    {
        Modbus_Meter_Config_Parameter_t sModbusMeterConfig;

        if (u16RxPayloadLen <= 1)
        {
            bFailFlag = true;
            break;
        }

        if (!app_storage_get_modbus_meter_config(pRxPayload[0], &sModbusMeterConfig))
        {
            bFailFlag = true;
            break;
        }

        for (uint16_t i = 1; i < u16RxPayloadLen; i++)
        {
            *p++ = pRxPayload[i];

            switch (pRxPayload[i])
            {
            case CONFIG_MODBUS_METER_SERIAL_NUMBER:
                memcpy(p, &sModbusMeterConfig.u8Serial, METER_SERIAL_SIZE);
                p += METER_SERIAL_SIZE;
                break;

            case CONFIG_MODBUS_METER_SLAVE_ADDRESS:
                memcpy(p, &sModbusMeterConfig.sConfig.sModbusConfig.u8SlaveAddress, sizeof(sModbusMeterConfig.sConfig.sModbusConfig.u8SlaveAddress));
                p += sizeof(sModbusMeterConfig.sConfig.sModbusConfig.u8SlaveAddress);
                break;

            case CONFIG_MODBUS_METER_BAUDRATE:
                memcpy(p, &sModbusMeterConfig.sConfig.sModbusConfig.u32BaudRate, sizeof(sModbusMeterConfig.sConfig.sModbusConfig.u32BaudRate));
                p += sizeof(sModbusMeterConfig.sConfig.sModbusConfig.u32BaudRate);
                break;

            case CONFIG_MODBUS_METER_SERIAL_CONFIG:
                memcpy(p, &sModbusMeterConfig.sConfig.sModbusConfig.u8SerialConfig, sizeof(sModbusMeterConfig.sConfig.sModbusConfig.u8SerialConfig));
                p += sizeof(sModbusMeterConfig.sConfig.sModbusConfig.u8SerialConfig);
                break;

            case CONFIG_MODBUS_METER_READ_FUNC_CODE:
                memcpy(p, &sModbusMeterConfig.sConfig.sModbusConfig.u8ReadFuncCode, sizeof(sModbusMeterConfig.sConfig.sModbusConfig.u8ReadFuncCode));
                p += sizeof(sModbusMeterConfig.sConfig.sModbusConfig.u8ReadFuncCode);
                break;

            case CONFIG_MODBUS_METER_FORWARD_TOTAL_ENABLE:
                memcpy(p, &sModbusMeterConfig.sConfig.sForwardTotalizer.u8ParameterEnable, sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.u8ParameterEnable));
                p += sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.u8ParameterEnable);
                break;

            case CONFIG_MODBUS_METER_FORWARD_TOTAL_REG_ADDR:
                memcpy(p, &sModbusMeterConfig.sConfig.sForwardTotalizer.u16RegisterAddress, sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.u16RegisterAddress));
                p += sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.u16RegisterAddress);
                break;

            case CONFIG_MODBUS_METER_FORWARD_TOTAL_DATA_TYPE:
                memcpy(p, &sModbusMeterConfig.sConfig.sForwardTotalizer.u8DataType, sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.u8DataType));
                p += sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.u8DataType);
                break;

            case CONFIG_MODBUS_METER_FORWARD_TOTAL_WORD_SWAP:
                memcpy(p, &sModbusMeterConfig.sConfig.sForwardTotalizer.u8WordSwap, sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.u8WordSwap));
                p += sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.u8WordSwap);
                break;

            case CONFIG_MODBUS_METER_FORWARD_TOTAL_MULTIPLIER:
                memcpy(p, &sModbusMeterConfig.sConfig.sForwardTotalizer.s8Multiplier, sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.s8Multiplier));
                p += sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.s8Multiplier);
                break;

            case CONFIG_MODBUS_METER_REVERSE_TOTAL_ENABLE:
                memcpy(p, &sModbusMeterConfig.sConfig.sReverseTotalizer.u8ParameterEnable, sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.u8ParameterEnable));
                p += sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.u8ParameterEnable);
                break;

            case CONFIG_MODBUS_METER_REVERSE_TOTAL_REG_ADDR:
                memcpy(p, &sModbusMeterConfig.sConfig.sReverseTotalizer.u16RegisterAddress, sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.u16RegisterAddress));
                p += sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.u16RegisterAddress);
                break;

            case CONFIG_MODBUS_METER_REVERSE_TOTAL_DATA_TYPE:
                memcpy(p, &sModbusMeterConfig.sConfig.sReverseTotalizer.u8DataType, sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.u8DataType));
                p += sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.u8DataType);
                break;

            case CONFIG_MODBUS_METER_REVERSE_TOTAL_WORD_SWAP:
                memcpy(p, &sModbusMeterConfig.sConfig.sReverseTotalizer.u8WordSwap, sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.u8WordSwap));
                p += sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.u8WordSwap);
                break;

            case CONFIG_MODBUS_METER_REVERSE_TOTAL_MULTIPLIER:
                memcpy(p, &sModbusMeterConfig.sConfig.sReverseTotalizer.s8Multiplier, sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.s8Multiplier));
                p += sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.s8Multiplier);
                break;

            case CONFIG_MODBUS_METER_FLOW_RATE_ENABLE:
                memcpy(p, &sModbusMeterConfig.sConfig.sFlowRate.u8ParameterEnable, sizeof(sModbusMeterConfig.sConfig.sFlowRate.u8ParameterEnable));
                p += sizeof(sModbusMeterConfig.sConfig.sFlowRate.u8ParameterEnable);
                break;

            case CONFIG_MODBUS_METER_FLOW_RATE_TOTAL_REG_ADDR:
                memcpy(p, &sModbusMeterConfig.sConfig.sFlowRate.u16RegisterAddress, sizeof(sModbusMeterConfig.sConfig.sFlowRate.u16RegisterAddress));
                p += sizeof(sModbusMeterConfig.sConfig.sFlowRate.u16RegisterAddress);
                break;

            case CONFIG_MODBUS_METER_FLOW_RATE_TOTAL_DATA_TYPE:
                memcpy(p, &sModbusMeterConfig.sConfig.sFlowRate.u8DataType, sizeof(sModbusMeterConfig.sConfig.sFlowRate.u8DataType));
                p += sizeof(sModbusMeterConfig.sConfig.sFlowRate.u8DataType);
                break;

            case CONFIG_MODBUS_METER_FLOW_RATE_TOTAL_WORD_SWAP:
                memcpy(p, &sModbusMeterConfig.sConfig.sFlowRate.u8WordSwap, sizeof(sModbusMeterConfig.sConfig.sFlowRate.u8WordSwap));
                p += sizeof(sModbusMeterConfig.sConfig.sFlowRate.u8WordSwap);
                break;

            case CONFIG_MODBUS_METER_FLOW_RATE_TOTAL_MULTIPLIER:
                memcpy(p, &sModbusMeterConfig.sConfig.sFlowRate.s8Multiplier, sizeof(sModbusMeterConfig.sConfig.sFlowRate.s8Multiplier));
                p += sizeof(sModbusMeterConfig.sConfig.sFlowRate.s8Multiplier);
                break;

            default:
                bFailFlag = true;
                break;
            }
        }

        break;
    }

    case CONFIG_PRESSURE_SENSOR:
    {
        Pressure_Sensor_Config_Parameter_t sPressureSensorConfig;

        if (u16RxPayloadLen <= 1)
        {
            bFailFlag = true;
            break;
        }

        if (!app_storage_get_pressure_sensor_config(pRxPayload[0], &sPressureSensorConfig))
        {
            bFailFlag = true;
            break;
        }

        for (uint16_t i = 1; i < u16RxPayloadLen; i++)
        {
            *p++ = pRxPayload[i];

            switch (pRxPayload[i])
            {
            case CONFIG_PRESSURE_SENSOR_SERIAL_NUMBER:
                memcpy(p, &sPressureSensorConfig.u8Serial, METER_SERIAL_SIZE);
                p += METER_SERIAL_SIZE;
                break;

            case CONFIG_PRESSURE_SENSOR_MIN_CURRENT:
                memcpy(p, &sPressureSensorConfig.sConfig.fMinCurrent, sizeof(sPressureSensorConfig.sConfig.fMinCurrent));
                p += sizeof(sPressureSensorConfig.sConfig.fMinCurrent);
                break;

            case CONFIG_PRESSURE_SENSOR_MAX_CURRENT:
                memcpy(p, &sPressureSensorConfig.sConfig.fMaxCurrent, sizeof(sPressureSensorConfig.sConfig.fMaxCurrent));
                p += sizeof(sPressureSensorConfig.sConfig.fMaxCurrent);
                break;

            case CONFIG_PRESSURE_SENSOR_MIN_PRESSURE:
                memcpy(p, &sPressureSensorConfig.sConfig.fMinPressure, sizeof(sPressureSensorConfig.sConfig.fMinPressure));
                p += sizeof(sPressureSensorConfig.sConfig.fMinPressure);
                break;

            case CONFIG_PRESSURE_SENSOR_MAX_PRESSURE:
                memcpy(p, &sPressureSensorConfig.sConfig.fMaxPressure, sizeof(sPressureSensorConfig.sConfig.fMaxPressure));
                p += sizeof(sPressureSensorConfig.sConfig.fMaxPressure);
                break;

            default:
                bFailFlag = true;
                break;
            }
        }

        break;
    }

    case CONFIG_MCU_RESET_COUNT:
    {
        uint8_t u8ResetCount = app_storage_get_reset_count();
        memcpy(p, &u8ResetCount, sizeof(u8ResetCount));
        p += sizeof(u8ResetCount);
        break;
    }

    default:
        app_protocol_pack_ack(CMD_GET, u8Id, PROTOCOL_ERR_CMD_INVALID, pTxFrame, u16TxFrameLen);
        return PROTOCOL_ERR_CMD_INVALID;
    }

    if (bFailFlag)
    {
        app_protocol_pack_ack(CMD_GET, u8Id, PROTOCOL_ERR_FRAME_INVALID, pTxFrame, u16TxFrameLen);
        return PROTOCOL_ERR_FRAME_INVALID;
    }

    sv_protocol_pack(true, app_storage_get_module_serial(), CMD_GET, u8Id, au8TxPayload, p - au8TxPayload, pTxFrame, u16TxFrameLen);
    return PROTOCOL_ERR_SUCCESS;
}

static Protocol_Err_Code_t app_protocol_set_handler(uint8_t u8Id, const uint8_t *pRxPayload, uint16_t u16RxPayloadLen, uint8_t *pTxFrame, uint16_t *u16TxFrameLen)
{
    bool bFailFlag = false;
    const uint8_t *p = pRxPayload;

    switch (u8Id)
    {
    case CONFIG_MODULE_SERIAL:
    {
        uint64_t u64ModuleSerial;

        if (u16RxPayloadLen != sizeof(u64ModuleSerial))
        {
            bFailFlag = true;
            break;
        }

        memcpy(&u64ModuleSerial, pRxPayload, sizeof(u64ModuleSerial));
        app_storage_set_module_serial(u64ModuleSerial);
        break;
    }

    case CONFIG_TIME:
    {
        Date_Time_t sDateTime;

        if (u16RxPayloadLen != sizeof(sDateTime))
        {
            bFailFlag = true;
            break;
        }

        memcpy(&sDateTime, pRxPayload, sizeof(sDateTime));
        sv_time_set_date_time(&sDateTime);
        break;
    }

    case CONFIG_IP_ENDPOINT:
    {
        Ip_Endpoint_t sIpEndpoint;

        if (u16RxPayloadLen != sizeof(sIpEndpoint))
        {
            bFailFlag = true;
            break;
        }

        memcpy(&sIpEndpoint, pRxPayload, sizeof(sIpEndpoint));
        app_storage_set_ip_endpoint(&sIpEndpoint);
        break;
    }

    case CONFIG_MODULE:
    {
        Module_Config_Parameter_t sModuleConfig;

        if (u16RxPayloadLen == 0)
        {
            bFailFlag = true;
            break;
        }

        if (!app_storage_get_module_config(&sModuleConfig))
        {
            bFailFlag = true;
            break;
        }

        while ((uint16_t)(p - pRxPayload) < u16RxPayloadLen)
        {
            switch (*p++)
            {
            case CONFIG_MODULE_LATCH_PERIOD:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModuleConfig.u16LatchPeriod) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModuleConfig.u16LatchPeriod, p, sizeof(sModuleConfig.u16LatchPeriod));
                p += sizeof(sModuleConfig.u16LatchPeriod);
                break;

            case CONFIG_MODULE_PUSH_PERIOD:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModuleConfig.u16PushPeriod) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModuleConfig.u16PushPeriod, p, sizeof(sModuleConfig.u16PushPeriod));
                p += sizeof(sModuleConfig.u16PushPeriod);
                break;

            case CONFIG_MODULE_TIMEZONE:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModuleConfig.fTimezone) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModuleConfig.fTimezone, p, sizeof(sModuleConfig.fTimezone));
                p += sizeof(sModuleConfig.fTimezone);
                break;

            default:
                bFailFlag = true;
                break;
            }

            if (bFailFlag)
            {
                break;
            }
        }

        if (!bFailFlag)
        {
            app_storage_set_module_config(&sModuleConfig);
        }

        break;
    }

    case CONFIG_PULSE_METER:
    {
        Pulse_Meter_Config_Parameter_t sPulseMeterConfig;

        if (u16RxPayloadLen <= 1)
        {
            bFailFlag = true;
            break;
        }

        if (!app_storage_get_pulse_meter_config(*p++, &sPulseMeterConfig))
        {
            bFailFlag = true;
            break;
        }

        while ((uint16_t)(p - pRxPayload) < u16RxPayloadLen)
        {
            switch (*p++)
            {
            case CONFIG_PULSE_METER_SERIAL_NUMBER:
                if ((uint16_t)(p - pRxPayload) + METER_SERIAL_SIZE > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sPulseMeterConfig.u8Serial, p, METER_SERIAL_SIZE);
                p += METER_SERIAL_SIZE;
                break;

            case CONFIG_PULSE_METER_PULSE_FACTOR:
                if ((uint16_t)(p - pRxPayload) + sizeof(sPulseMeterConfig.sConfig.u16PulseFactor) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sPulseMeterConfig.sConfig.u16PulseFactor, p, sizeof(sPulseMeterConfig.sConfig.u16PulseFactor));
                p += sizeof(sPulseMeterConfig.sConfig.u16PulseFactor);
                break;

            case CONFIG_PULSE_METER_PIN1:
                if ((uint16_t)(p - pRxPayload) + sizeof(sPulseMeterConfig.sConfig.sPulseConfig.u8Pin1Select) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sPulseMeterConfig.sConfig.sPulseConfig.u8Pin1Select, p, sizeof(sPulseMeterConfig.sConfig.sPulseConfig.u8Pin1Select));
                p += sizeof(sPulseMeterConfig.sConfig.sPulseConfig.u8Pin1Select);
                break;

            case CONFIG_PULSE_METER_PIN2:
                if ((uint16_t)(p - pRxPayload) + sizeof(sPulseMeterConfig.sConfig.sPulseConfig.u8Pin2Select) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sPulseMeterConfig.sConfig.sPulseConfig.u8Pin2Select, p, sizeof(sPulseMeterConfig.sConfig.sPulseConfig.u8Pin2Select));
                p += sizeof(sPulseMeterConfig.sConfig.sPulseConfig.u8Pin2Select);
                break;

            case CONFIG_PULSE_METER_PULSE_TYPE:
                if ((uint16_t)(p - pRxPayload) + sizeof(sPulseMeterConfig.sConfig.sPulseConfig.u8PulseType) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sPulseMeterConfig.sConfig.sPulseConfig.u8PulseType, p, sizeof(sPulseMeterConfig.sConfig.sPulseConfig.u8PulseType));
                p += sizeof(sPulseMeterConfig.sConfig.sPulseConfig.u8PulseType);
                break;

            case CONFIG_PULSE_METER_EDGE_TYPE:
                if ((uint16_t)(p - pRxPayload) + sizeof(sPulseMeterConfig.sConfig.sPulseConfig.u8EdgeType) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sPulseMeterConfig.sConfig.sPulseConfig.u8EdgeType, p, sizeof(sPulseMeterConfig.sConfig.sPulseConfig.u8EdgeType));
                p += sizeof(sPulseMeterConfig.sConfig.sPulseConfig.u8EdgeType);
                break;

            default:
                bFailFlag = true;
                break;
            }

            if (bFailFlag)
            {
                break;
            }
        }

        if (!bFailFlag)
        {
            app_storage_set_pulse_meter_config(pRxPayload[0], &sPulseMeterConfig);
        }

        break;
    }

    case CONFIG_MODBUS_METER:
    {
        Modbus_Meter_Config_Parameter_t sModbusMeterConfig;

        if (u16RxPayloadLen <= 1)
        {
            bFailFlag = true;
            break;
        }

        if (!app_storage_get_modbus_meter_config(*p++, &sModbusMeterConfig))
        {
            bFailFlag = true;
            break;
        }

        while ((uint16_t)(p - pRxPayload) < u16RxPayloadLen)
        {
            switch (*p++)
            {
            case CONFIG_MODBUS_METER_SERIAL_NUMBER:
                if ((uint16_t)(p - pRxPayload) + METER_SERIAL_SIZE > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModbusMeterConfig.u8Serial, p, METER_SERIAL_SIZE);
                p += METER_SERIAL_SIZE;
                break;

            case CONFIG_MODBUS_METER_SLAVE_ADDRESS:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModbusMeterConfig.sConfig.sModbusConfig.u8SlaveAddress) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModbusMeterConfig.sConfig.sModbusConfig.u8SlaveAddress, p, sizeof(sModbusMeterConfig.sConfig.sModbusConfig.u8SlaveAddress));
                p += sizeof(sModbusMeterConfig.sConfig.sModbusConfig.u8SlaveAddress);
                break;

            case CONFIG_MODBUS_METER_BAUDRATE:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModbusMeterConfig.sConfig.sModbusConfig.u32BaudRate) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModbusMeterConfig.sConfig.sModbusConfig.u32BaudRate, p, sizeof(sModbusMeterConfig.sConfig.sModbusConfig.u32BaudRate));
                p += sizeof(sModbusMeterConfig.sConfig.sModbusConfig.u32BaudRate);
                break;

            case CONFIG_MODBUS_METER_SERIAL_CONFIG:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModbusMeterConfig.sConfig.sModbusConfig.u8SerialConfig) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModbusMeterConfig.sConfig.sModbusConfig.u8SerialConfig, p, sizeof(sModbusMeterConfig.sConfig.sModbusConfig.u8SerialConfig));
                p += sizeof(sModbusMeterConfig.sConfig.sModbusConfig.u8SerialConfig);
                break;

            case CONFIG_MODBUS_METER_READ_FUNC_CODE:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModbusMeterConfig.sConfig.sModbusConfig.u8ReadFuncCode) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModbusMeterConfig.sConfig.sModbusConfig.u8ReadFuncCode, p, sizeof(sModbusMeterConfig.sConfig.sModbusConfig.u8ReadFuncCode));
                p += sizeof(sModbusMeterConfig.sConfig.sModbusConfig.u8ReadFuncCode);
                break;

            case CONFIG_MODBUS_METER_FORWARD_TOTAL_ENABLE:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.u8ParameterEnable) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModbusMeterConfig.sConfig.sForwardTotalizer.u8ParameterEnable, p, sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.u8ParameterEnable));
                p += sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.u8ParameterEnable);
                break;

            case CONFIG_MODBUS_METER_FORWARD_TOTAL_REG_ADDR:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.u16RegisterAddress) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModbusMeterConfig.sConfig.sForwardTotalizer.u16RegisterAddress, p, sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.u16RegisterAddress));
                p += sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.u16RegisterAddress);
                break;

            case CONFIG_MODBUS_METER_FORWARD_TOTAL_DATA_TYPE:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.u8DataType) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModbusMeterConfig.sConfig.sForwardTotalizer.u8DataType, p, sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.u8DataType));
                p += sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.u8DataType);
                break;

            case CONFIG_MODBUS_METER_FORWARD_TOTAL_WORD_SWAP:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.u8WordSwap) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModbusMeterConfig.sConfig.sForwardTotalizer.u8WordSwap, p, sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.u8WordSwap));
                p += sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.u8WordSwap);
                break;

            case CONFIG_MODBUS_METER_FORWARD_TOTAL_MULTIPLIER:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.s8Multiplier) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModbusMeterConfig.sConfig.sForwardTotalizer.s8Multiplier, p, sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.s8Multiplier));
                p += sizeof(sModbusMeterConfig.sConfig.sForwardTotalizer.s8Multiplier);
                break;

            case CONFIG_MODBUS_METER_REVERSE_TOTAL_ENABLE:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.u8ParameterEnable) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModbusMeterConfig.sConfig.sReverseTotalizer.u8ParameterEnable, p, sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.u8ParameterEnable));
                p += sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.u8ParameterEnable);
                break;

            case CONFIG_MODBUS_METER_REVERSE_TOTAL_REG_ADDR:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.u16RegisterAddress) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModbusMeterConfig.sConfig.sReverseTotalizer.u16RegisterAddress, p, sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.u16RegisterAddress));
                p += sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.u16RegisterAddress);
                break;

            case CONFIG_MODBUS_METER_REVERSE_TOTAL_DATA_TYPE:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.u8DataType) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModbusMeterConfig.sConfig.sReverseTotalizer.u8DataType, p, sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.u8DataType));
                p += sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.u8DataType);
                break;

            case CONFIG_MODBUS_METER_REVERSE_TOTAL_WORD_SWAP:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.u8WordSwap) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModbusMeterConfig.sConfig.sReverseTotalizer.u8WordSwap, p, sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.u8WordSwap));
                p += sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.u8WordSwap);
                break;

            case CONFIG_MODBUS_METER_REVERSE_TOTAL_MULTIPLIER:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.s8Multiplier) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModbusMeterConfig.sConfig.sReverseTotalizer.s8Multiplier, p, sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.s8Multiplier));
                p += sizeof(sModbusMeterConfig.sConfig.sReverseTotalizer.s8Multiplier);
                break;

            case CONFIG_MODBUS_METER_FLOW_RATE_ENABLE:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModbusMeterConfig.sConfig.sFlowRate.u8ParameterEnable) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModbusMeterConfig.sConfig.sFlowRate.u8ParameterEnable, p, sizeof(sModbusMeterConfig.sConfig.sFlowRate.u8ParameterEnable));
                p += sizeof(sModbusMeterConfig.sConfig.sFlowRate.u8ParameterEnable);
                break;

            case CONFIG_MODBUS_METER_FLOW_RATE_TOTAL_REG_ADDR:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModbusMeterConfig.sConfig.sFlowRate.u16RegisterAddress) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModbusMeterConfig.sConfig.sFlowRate.u16RegisterAddress, p, sizeof(sModbusMeterConfig.sConfig.sFlowRate.u16RegisterAddress));
                p += sizeof(sModbusMeterConfig.sConfig.sFlowRate.u16RegisterAddress);
                break;

            case CONFIG_MODBUS_METER_FLOW_RATE_TOTAL_DATA_TYPE:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModbusMeterConfig.sConfig.sFlowRate.u8DataType) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModbusMeterConfig.sConfig.sFlowRate.u8DataType, p, sizeof(sModbusMeterConfig.sConfig.sFlowRate.u8DataType));
                p += sizeof(sModbusMeterConfig.sConfig.sFlowRate.u8DataType);
                break;

            case CONFIG_MODBUS_METER_FLOW_RATE_TOTAL_WORD_SWAP:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModbusMeterConfig.sConfig.sFlowRate.u8WordSwap) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModbusMeterConfig.sConfig.sFlowRate.u8WordSwap, p, sizeof(sModbusMeterConfig.sConfig.sFlowRate.u8WordSwap));
                p += sizeof(sModbusMeterConfig.sConfig.sFlowRate.u8WordSwap);
                break;

            case CONFIG_MODBUS_METER_FLOW_RATE_TOTAL_MULTIPLIER:
                if ((uint16_t)(p - pRxPayload) + sizeof(sModbusMeterConfig.sConfig.sFlowRate.s8Multiplier) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sModbusMeterConfig.sConfig.sFlowRate.s8Multiplier, p, sizeof(sModbusMeterConfig.sConfig.sFlowRate.s8Multiplier));
                p += sizeof(sModbusMeterConfig.sConfig.sFlowRate.s8Multiplier);
                break;

            default:
                bFailFlag = true;
                break;
            }

            if (bFailFlag)
            {
                break;
            }
        }

        if (!bFailFlag)
        {
            app_storage_set_modbus_meter_config(pRxPayload[0], &sModbusMeterConfig);
        }

        break;
    }

    case CONFIG_PRESSURE_SENSOR:
    {
        Pressure_Sensor_Config_Parameter_t sPressureSensorConfig;

        if (u16RxPayloadLen <= 1)
        {
            bFailFlag = true;
            break;
        }

        if (!app_storage_get_pressure_sensor_config(*p++, &sPressureSensorConfig))
        {
            bFailFlag = true;
            break;
        }

        while ((uint16_t)(p - pRxPayload) < u16RxPayloadLen)
        {
            switch (*p++)
            {
            case CONFIG_PRESSURE_SENSOR_SERIAL_NUMBER:
                if ((uint16_t)(p - pRxPayload) + METER_SERIAL_SIZE > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sPressureSensorConfig.u8Serial, p, METER_SERIAL_SIZE);
                p += METER_SERIAL_SIZE;
                break;

            case CONFIG_PRESSURE_SENSOR_MIN_CURRENT:
                if ((uint16_t)(p - pRxPayload) + sizeof(sPressureSensorConfig.sConfig.fMinCurrent) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sPressureSensorConfig.sConfig.fMinCurrent, p, sizeof(sPressureSensorConfig.sConfig.fMinCurrent));
                p += sizeof(sPressureSensorConfig.sConfig.fMinCurrent);
                break;

            case CONFIG_PRESSURE_SENSOR_MAX_CURRENT:
                if ((uint16_t)(p - pRxPayload) + sizeof(sPressureSensorConfig.sConfig.fMaxCurrent) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sPressureSensorConfig.sConfig.fMaxCurrent, p, sizeof(sPressureSensorConfig.sConfig.fMaxCurrent));
                p += sizeof(sPressureSensorConfig.sConfig.fMaxCurrent);
                break;

            case CONFIG_PRESSURE_SENSOR_MIN_PRESSURE:
                if ((uint16_t)(p - pRxPayload) + sizeof(sPressureSensorConfig.sConfig.fMinPressure) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sPressureSensorConfig.sConfig.fMinPressure, p, sizeof(sPressureSensorConfig.sConfig.fMinPressure));
                p += sizeof(sPressureSensorConfig.sConfig.fMinPressure);
                break;

            case CONFIG_PRESSURE_SENSOR_MAX_PRESSURE:
                if ((uint16_t)(p - pRxPayload) + sizeof(sPressureSensorConfig.sConfig.fMaxPressure) > u16RxPayloadLen)
                {
                    bFailFlag = true;
                    break;
                }
                memcpy(&sPressureSensorConfig.sConfig.fMaxPressure, p, sizeof(sPressureSensorConfig.sConfig.fMaxPressure));
                p += sizeof(sPressureSensorConfig.sConfig.fMaxPressure);
                break;

            default:
                bFailFlag = true;
                break;
            }

            if (bFailFlag)
            {
                break;
            }
        }

        if (!bFailFlag)
        {
            app_storage_set_pressure_sensor_config(pRxPayload[0], &sPressureSensorConfig);
        }

        break;
    }

    case CONFIG_REBOOT:
        bRebbotFlag = true;
        u32TimeRequest = sys_time_ms();
        break;

    case CONFIG_RESET_SETTING:
        bSettingResetRequestFlag = true;
        u32TimeRequest = sys_time_ms();
        break;

    case CONFIG_CHANGE_PASSWORD:
    {
        uint8_t au8Password[PASSWORD_LENGTH];

        if (u16RxPayloadLen != PASSWORD_LENGTH + 1)
        {
            bFailFlag = true;
            break;
        }

        memcpy(au8Password, &pRxPayload[1], PASSWORD_LENGTH);
        app_storage_set_password(pRxPayload[0], au8Password);

        break;
    }

    case CONFIG_RESET_PASSWORD:
        app_storage_restore_default_password();
        break;

    case CONFIG_MCU_RESET_COUNT:
    {
        uint8_t u8ResetCount;

        if (u16RxPayloadLen != sizeof(u8ResetCount))
        {
            bFailFlag = true;
            break;
        }

        memcpy(&u8ResetCount, pRxPayload, sizeof(u8ResetCount));
        app_storage_set_reset_count(u8ResetCount);
        break;
    }

    case CONFIG_ERASE_MEASUREMENT_DATA:
        app_storage_latch_clear();
        break;

    case CONFIG_ERASE_EVENT_DATA:
        app_storage_event_clear();
        break;

    case CONFIG_ERASE_LOG_DATA:
        app_storage_log_clear();
        break;

    case CONFIG_FACTORY_RESET:
        bFactoryResetRequestFlag = true;
        u32TimeRequest = sys_time_ms();
        break;

    case CONFIG_LATCH_IMMEDIATELY:
        app_latch_activate();
        break;

    case CONFIG_PUSH_IMMEDIATELY:
        app_cellular_push_activate();
        break;

    case CONFIG_EVENT_CREATE:
    {
        Event_Data_t sEvent;

        if (u16RxPayloadLen != sizeof(sEvent) - sizeof(sEvent.sDateTime))
        {
            bFailFlag = true;
            break;
        }

        sv_time_get_date_time(&sEvent.sDateTime);
        sEvent.u8MeterType = pRxPayload[0];
        sEvent.u8MeterIndex = pRxPayload[1];
        sEvent.u8EventCode = pRxPayload[2];
        app_storage_event_save(&sEvent);
        break;
    }

    default:
        app_protocol_pack_ack(CMD_SET, u8Id, PROTOCOL_ERR_FRAME_INVALID, pTxFrame, u16TxFrameLen);
        return PROTOCOL_ERR_FRAME_INVALID;
    }

    if (bFailFlag)
    {
        app_protocol_pack_ack(CMD_SET, u8Id, PROTOCOL_ERR_FRAME_INVALID, pTxFrame, u16TxFrameLen);
        return PROTOCOL_ERR_FRAME_INVALID;
    }

    app_protocol_pack_ack(CMD_SET, u8Id, PROTOCOL_ERR_SUCCESS, pTxFrame, u16TxFrameLen);
    return PROTOCOL_ERR_SUCCESS;
}

static Protocol_Err_Code_t app_protocol_query_handler(uint8_t u8Id, const uint8_t *pRxPayload, uint16_t u16RxPayloadLen, uint8_t *pTxFrame, uint16_t *u16TxFrameLen)
{
    bool bFailFlag = false;
    uint8_t au8TxPayload[1024];
    uint8_t *p = au8TxPayload;

    switch (u8Id)
    {
    case QUERY_BOOTLOADER_VERSION:
        // app_storage_get_boot_version(p);
        // p += VERSION_SIZE;
        break;

    case QUERY_FIRMWARE_VERSION:
        // app_storage_get_fw_version(p);
        // p += VERSION_SIZE;
        break;

    case QUERY_POWER_SUPPLY_INFO:
        break;

    case QUERY_SIM_NETWORK_INFO:
    {
        uint8_t au8Ccid[SIM_CCID_SIZE] = {0};
        uint8_t u8CcidLen;
        int8_t s8Rssi;
        int8_t s8Rsrp;
        int8_t s8Rsrq;
        int8_t s8Rssnr;
        uint32_t u32TimeTaskStart = sys_time_ms();

        bool bGetCcidStatus = false;
        bool bGetSignalQualityStatus = false;

        app_cellular_start();

        while (sys_time_ms() - u32TimeTaskStart < CELLULAR_GET_INFO_TIMEOUT)
        {
            app_cellular_execute();

            if (!bGetCcidStatus)
            {
                if (app_cellular_get_ccid(au8Ccid, &u8CcidLen) == TASK_STATUS_SUCCESS)
                {
                    bGetCcidStatus = true;
                }
            }

            if (!bGetSignalQualityStatus)
            {
                if (app_cellular_get_signal_quality(&s8Rssi, &s8Rsrp, &s8Rsrq, &s8Rssnr) == TASK_STATUS_SUCCESS)
                {
                    bGetSignalQualityStatus = true;
                }
            }

            if (bGetCcidStatus && bGetSignalQualityStatus)
            {
                app_cellular_stop();
                break;
            }
        }

        if (bGetCcidStatus)
        {
            memcpy(p, au8Ccid, SIM_CCID_SIZE);
        }
        else
        {
            memset(p, 0, SIM_CCID_SIZE);
        }
        p += SIM_CCID_SIZE;

        if (bGetSignalQualityStatus)
        {
            *p++ = s8Rssi;
            *p++ = s8Rsrp;
            *p++ = s8Rsrq;
            *p++ = s8Rssnr;
        }
        else
        {
            memset(p, 0, 4);
            p += 4;
        }

        break;
    }

    case QUERY_PULSE_METER_DATA:
    {
        uint8_t u8MeterIndex = pRxPayload[0];
        uint8_t au8MeterSerial[METER_SERIAL_SIZE];
        Pulse_Meter_Data_t sPulseMeterData;

        app_storage_get_meter_serial(PULSE_METER_TYPE, u8MeterIndex, au8MeterSerial);
        memcpy(p, au8MeterSerial, METER_SERIAL_SIZE);
        p += METER_SERIAL_SIZE;

        if (!sv_pulse_meter_get_data(u8MeterIndex, &sPulseMeterData))
        {
            sPulseMeterData.dTotalForward = -1;
            sPulseMeterData.dTotalReverse = -1;
            sPulseMeterData.dFlowRate = -1;
        }
        memcpy(p, &sPulseMeterData, sizeof(sPulseMeterData));
        p += sizeof(sPulseMeterData);

        break;
    }

    case QUERY_MODBUS_METER_DATA:
    {
        uint8_t u8MeterIndex = pRxPayload[0];
        uint8_t au8MeterSerial[METER_SERIAL_SIZE];
        Modbus_Meter_Data_t sModbusMeterData;

        app_storage_get_meter_serial(MODBUS_METER_TYPE, u8MeterIndex, au8MeterSerial);
        memcpy(p, au8MeterSerial, METER_SERIAL_SIZE);
        p += METER_SERIAL_SIZE;

        if (!sv_modbus_meter_get_data(u8MeterIndex, &sModbusMeterData))
        {
            sModbusMeterData.dTotalForward = -1;
            sModbusMeterData.dTotalReverse = -1;
            sModbusMeterData.dFlowRate = -1;
        }
        memcpy(p, &sModbusMeterData, sizeof(sModbusMeterData));
        p += sizeof(sModbusMeterData);

        break;
    }

    case QUERY_PRESSURE_SENSOR_DATA:
    {
        uint8_t u8MeterIndex = pRxPayload[0];
        uint8_t au8MeterSerial[METER_SERIAL_SIZE];
        Pressure_Sensor_Data_t sPressureSensorData;

        app_storage_get_meter_serial(PRESSURE_SENSOR_TYPE, u8MeterIndex, au8MeterSerial);
        memcpy(p, au8MeterSerial, METER_SERIAL_SIZE);
        p += METER_SERIAL_SIZE;

        if (!sv_pressure_sensor_get_data(u8MeterIndex, &sPressureSensorData))
        {
            sPressureSensorData.fPressure = -1;
        }
        memcpy(p, &sPressureSensorData, sizeof(sPressureSensorData));
        p += sizeof(sPressureSensorData);

        break;
    }

    case QUERY_LATCH:
    {
        Latch_Data_t sLatchData;
        uint16_t u16LatchIndex;
        uint16_t u16TxPayloadLen;

        if (u16RxPayloadLen != sizeof(u16LatchIndex))
        {
            bFailFlag = true;
            break;
        }

        memcpy(&u16LatchIndex, pRxPayload, sizeof(u16LatchIndex));

        if (!app_storage_latch_load_latest(u16LatchIndex, &sLatchData))
        {
            app_protocol_pack_ack(CMD_QUERY, u8Id, PROTOCOL_ERR_UNKNOW, pTxFrame, u16TxFrameLen);
            return PROTOCOL_ERR_UNKNOW;
        }

        app_protocol_pack_latch_payload(&sLatchData, au8TxPayload, &u16TxPayloadLen);
        p = au8TxPayload + u16TxPayloadLen;

        break;
    }

    case QUERY_EVENT:
    {
        Event_Data_t sEventData;
        uint16_t u16EventIndex;
        uint16_t u16TxPayloadLen;

        if (u16RxPayloadLen != sizeof(u16EventIndex))
        {
            bFailFlag = true;
            break;
        }

        memcpy(&u16EventIndex, pRxPayload, sizeof(u16EventIndex));

        if (!app_storage_event_load_latest(u16EventIndex, &sEventData))
        {
            app_protocol_pack_ack(CMD_QUERY, u8Id, PROTOCOL_ERR_UNKNOW, pTxFrame, u16TxFrameLen);
            return PROTOCOL_ERR_UNKNOW;
        }

        app_protocol_pack_event_payload(&sEventData, 1, au8TxPayload, &u16TxPayloadLen);
        p = au8TxPayload + u16TxPayloadLen;

        break;
    }

    case QUERY_PUSH_STATUS:
        break;

    case QUERY_LOG:
        break;

    default:
        bFailFlag = true;
        break;
    }

    if (bFailFlag)
    {
        app_protocol_pack_ack(CMD_QUERY, u8Id, PROTOCOL_ERR_FRAME_INVALID, pTxFrame, u16TxFrameLen);
        return PROTOCOL_ERR_FRAME_INVALID;
    }

    sv_protocol_pack(true, app_storage_get_module_serial(), CMD_QUERY, u8Id, au8TxPayload, p - au8TxPayload, pTxFrame, u16TxFrameLen);
    return PROTOCOL_ERR_SUCCESS;
}