#include "app_storage.h"

static uint64_t u64ModuleSerial;
static Device_Password_t sDevicePassword;

static Eeprom_Metadata_t sEepromMetadata;
static Eeprom_Metadata_Area_t eNextEepromMetadataLocation = EEPROM_METADATA_AREA_1;

static Eeprom_Runtime_Data_t sEepromRuntimeData;
static Eeprom_Runtime_Data_Area_t eNextEepromRuntimeDataLocation = EEPROM_RUNTIME_DATA_AREA_1;

static bool bPasswordChanged = false;
static bool bConfigChanged = false;

static Ip_Endpoint_t sIpEndpoint;
static Module_Config_Parameter_t sModuleConfig;
static Charge_Config_Parameter_t sChargeConfig;
static Pulse_Meter_Config_Parameter_t sPulseMeterConfig[MAX_PULSE_METER_COUNT];
static Modbus_Meter_Config_Parameter_t sModbusMeterConfig[MAX_MODBUS_METER_COUNT];
static Pressure_Sensor_Config_Parameter_t sPressureSensorConfig[MAX_PRESSURE_SENSOR_COUNT];

/*==================================================================================================
*                                PRIVATE FUNCTIONS DECLARATIONS
==================================================================================================*/

/**
 * @brief Initialize EEPROM header.
 */
static void app_storage_eeprom_header_init(void);

/**
 * @brief Initialize EEPROM metadata.
 */
static void app_storage_eeprom_metadata_init(void);

/**
 * @brief Load EEPROM metadata.
 */
static void app_storage_eeprom_metadata_load(void);

/**
 * @brief Save EEPROM metadata.
 */
static void app_storage_eeprom_metadata_save();

/**
 * @brief Initialize EEPROM runtime data.
 */
static void app_storage_eeprom_runtime_data_init(void);

/**
 * @brief Load EEPROM runtime data.
 */
static void app_storage_eeprom_runtime_data_load(void);

/**
 * @brief Save EEPROM runtime data.
 */
static void app_storage_eeprom_runtime_data_save(void);

/**
 * @brief Load module serial number from EEPROM.
 */
static void app_storage_module_serial_load(void);

/**
 * @brief Load password from EEPROM.
 */
static void app_storage_password_load(void);

/**
 * @brief Load configuration parameters.
 */
static void app_storage_config_parameter_load(void);

/*==================================================================================================
*                                   PUBLIC FUNCTIONS DEFINITIONS
==================================================================================================*/

void app_storage_init(void)
{
    sv_eeprom_init();

    Eeprom_Header_t sEepromHeader;
    bool bReadStatus = sv_eeprom_read(EEPROM_HEADER_ADDRESS, (uint8_t *)&sEepromHeader, sizeof(Eeprom_Header_t));

    if ((sEepromHeader.u32Magic != EEPROM_MAGIC_NUMBER) || (!bReadStatus))
    {
        app_storage_eeprom_metadata_init();
        app_storage_eeprom_runtime_data_init();
        app_storage_config_parameter_default();

        app_storage_restore_default_password();

        app_storage_eeprom_header_init();
    }
    else
    {
        app_storage_eeprom_metadata_load();
        app_storage_eeprom_runtime_data_load();
        app_storage_password_load();
        app_storage_config_parameter_load();
    }

    // Load module serial
    app_storage_module_serial_load();

    // Charge config
    sv_charge_init(sChargeConfig.u16IinLimMa, sChargeConfig.u16ChargeVoltageMv, sChargeConfig.u16ChargeCurrentMa, sChargeConfig.u8ChargeLedEnable != 0);

    // Load pulse count data to pulse meter
    Pulse_Count_t sPulseCount[MAX_PULSE_GATE_COUNT];
    memcpy(sPulseCount, sEepromRuntimeData.sPulseCount, sizeof(sPulseCount));
    sv_pulse_meter_count_init(sPulseCount);

    // Load config data to pulse meter
    for (uint8_t i = 0; i < MAX_PULSE_METER_COUNT; i++)
    {
        sv_pulse_meter_set_config(i, &sPulseMeterConfig->sConfig);
    }

    // Load config data to modbus meter
    for (uint8_t i = 0; i < MAX_MODBUS_METER_COUNT; i++)
    {
        sv_modbus_meter_set_config(i, &sModbusMeterConfig->sConfig);
    }

    // Load config data to pressure sensor
    for (uint8_t i = 0; i < MAX_PRESSURE_SENSOR_COUNT; i++)
    {
        sv_pressure_sensor_set_config(i, &sPressureSensorConfig->sConfig);
    }
}

void app_storage_header_invalidate(void)
{
    Eeprom_Header_t sEepromHeader;
    sEepromHeader.u32Magic = ~EEPROM_MAGIC_NUMBER;
    sv_eeprom_write(EEPROM_HEADER_ADDRESS, (const uint8_t *)&sEepromHeader, sizeof(Eeprom_Header_t));
}

void app_storage_get_bootloader_version(uint8_t *pData)
{
    sv_flash_read(BOOTLOADER_VERSION_ADDR, pData, VERSION_SIZE);
}

void app_storage_get_firmware_version(uint8_t *pData)
{
    uint8_t au8FirmwareVersion[VERSION_SIZE] = FIRMWARE_VERSION;
    memcpy(pData, au8FirmwareVersion, VERSION_SIZE);
}

bool app_storage_set_module_serial(uint64_t u64Serial)
{
    uint64_t u64Limit = 1ULL;

    for (uint8_t i = 0; i < MODULE_SERIAL_SIZE; i++)
    {
        u64Limit *= 10ULL;
    }

    if ((u64Serial < u64Limit / 10ULL) || (u64Serial >= u64Limit))
    {
        return false;
    }

    u64ModuleSerial = u64Serial;
    return sv_eeprom_write(EEPROM_DEVICE_SERIAL_ADDRESS, (const uint8_t *)&u64ModuleSerial, sizeof(u64ModuleSerial));
}

uint64_t app_storage_get_module_serial(void)
{
    return u64ModuleSerial;
}

bool app_storage_set_password(uint8_t u8PasswordLevel, const uint8_t *pPassword)
{
    if ((u8PasswordLevel > PASSWORD_COUNT) || (u8PasswordLevel == 0) || (pPassword == NULL))
    {
        return false;
    }

    if (memcmp(sDevicePassword.au8Passwords[u8PasswordLevel - 1], pPassword, PASSWORD_LENGTH) == 0)
    {
        return false;
    }

    bPasswordChanged = true;
    memcpy(sDevicePassword.au8Passwords[u8PasswordLevel - 1], pPassword, PASSWORD_LENGTH);
    return sv_eeprom_write(EEPROM_DEVICE_PASSWORD_ADDRESS, (const uint8_t *)&sDevicePassword, sizeof(Device_Password_t));
}

bool app_storage_get_password(uint8_t u8PasswordLevel, uint8_t *pPassword)
{
    if ((u8PasswordLevel > PASSWORD_COUNT) || (u8PasswordLevel == 0) || (pPassword == NULL))
    {
        return false;
    }

    memcpy(pPassword, sDevicePassword.au8Passwords[u8PasswordLevel - 1], PASSWORD_LENGTH);
    return true;
}

bool app_storage_restore_default_password(void)
{
    memcpy(sDevicePassword.au8Passwords[0], PASSWORD_DEFAULT_1, PASSWORD_LENGTH);
    memcpy(sDevicePassword.au8Passwords[1], PASSWORD_DEFAULT_2, PASSWORD_LENGTH);
    memcpy(sDevicePassword.au8Passwords[2], PASSWORD_DEFAULT_3, PASSWORD_LENGTH);

    return sv_eeprom_write(EEPROM_DEVICE_PASSWORD_ADDRESS, (const uint8_t *)&sDevicePassword, sizeof(Device_Password_t));
}

bool app_storage_get_password_changed_flag(void)
{
    if (bPasswordChanged)
    {
        bPasswordChanged = false;
        return true;
    }
    return false;
}

bool app_storage_get_config_changed_flag(void)
{
    if (bConfigChanged)
    {
        bConfigChanged = false;
        return true;
    }
    return false;
}

bool app_storage_set_ip_endpoint(const Ip_Endpoint_t *pIpEndpoint)
{
    memcpy(&sIpEndpoint, pIpEndpoint, sizeof(Ip_Endpoint_t));

    bConfigChanged = true;
    return sv_eeprom_write(EEPROM_IP_ENDPOINT_ADDRESS, (const uint8_t *)&sIpEndpoint, sizeof(Ip_Endpoint_t));
}

void app_storage_get_ip_endpoint(Ip_Endpoint_t *pIpEndpoint)
{
    memcpy(pIpEndpoint, &sIpEndpoint, sizeof(Ip_Endpoint_t));
}

uint16_t app_storage_get_latch_period(void)
{
    return sModuleConfig.u16LatchPeriod != 0 ? sModuleConfig.u16LatchPeriod : LATCH_PERIOD;
}

uint16_t app_storage_get_push_period(void)
{
    return sModuleConfig.u16PushPeriod != 0 ? sModuleConfig.u16PushPeriod : PUSH_PERIOD;
}

float app_storage_get_timezone(void)
{
    return sModuleConfig.fTimezone;
}

bool app_storage_event_save(const Event_Data_t *pEvent)
{
    if (pEvent == NULL)
    {
        return false;
    }

    uint32_t u32WriteAddress = EEPROM_EVENT_ADDRESS + (sEepromMetadata.u16NextEventSaveIndex * EVENT_PACKET_SIZE);

    if (!sv_eeprom_write(u32WriteAddress, (const uint8_t *)pEvent, sizeof(Event_Data_t)))
    {
        return false;
    }

    sEepromMetadata.u16NextEventSaveIndex++;

    if (sEepromMetadata.u16NextEventSaveIndex >= MAX_EVENT_COUNT)
    {
        sEepromMetadata.u16NextEventSaveIndex = 0;
    }

    if (sEepromMetadata.u16NextEventSaveIndex == sEepromMetadata.u16NextEventLoadIndex)
    {
        sEepromMetadata.u16NextEventLoadIndex++;
        if (sEepromMetadata.u16NextEventLoadIndex >= MAX_EVENT_COUNT)
        {
            sEepromMetadata.u16NextEventLoadIndex = 0;
        }
    }

    if (sEepromMetadata.u16EventCount < MAX_EVENT_COUNT)
    {
        sEepromMetadata.u16EventCount++;
    }

    app_storage_eeprom_metadata_save(&sEepromMetadata);

    return true;
}

bool app_storage_event_load_next(uint16_t u16EventIndex, Event_Data_t *pEvent)
{
    if ((pEvent == NULL) || (u16EventIndex >= sEepromMetadata.u16EventCount))
    {
        return false;
    }

    uint16_t u16EventSendIndex = (sEepromMetadata.u16NextEventLoadIndex + u16EventIndex) % MAX_EVENT_COUNT;

    if (u16EventSendIndex == sEepromMetadata.u16NextEventSaveIndex)
    {
        return false; // No new data available
    }

    uint32_t u32ReadAddress = EEPROM_EVENT_ADDRESS + (u16EventSendIndex * EVENT_PACKET_SIZE);
    return sv_eeprom_read(u32ReadAddress, (uint8_t *)pEvent, sizeof(Event_Data_t));
}

bool app_storage_event_load_latest(uint16_t u16EventIndex, Event_Data_t *pEvent)
{
    uint16_t u16EventReadIndex;
    uint32_t u32ReadAddress;

    if ((pEvent == NULL) || (u16EventIndex >= sEepromMetadata.u16EventCount))
    {
        return false;
    }

    u16EventReadIndex = (sEepromMetadata.u16NextEventSaveIndex + MAX_EVENT_COUNT - 1U - u16EventIndex) % MAX_EVENT_COUNT;

    u32ReadAddress = EEPROM_EVENT_ADDRESS + (u16EventReadIndex * EVENT_PACKET_SIZE);

    return sv_eeprom_read(u32ReadAddress, (uint8_t *)pEvent, sizeof(Event_Data_t));
}

void app_storage_event_clear(void)
{
    sEepromMetadata.u16NextEventLoadIndex = sEepromMetadata.u16NextEventSaveIndex;
    sEepromMetadata.u16EventCount = 0;

    app_storage_eeprom_metadata_save(&sEepromMetadata);
}

bool app_storage_push_status_save(const Push_Status_Data_t *pData)
{
    if (pData == NULL)
    {
        return false;
    }

    uint32_t u32WriteAddress = EEPROM_PUSH_STATUS_ADDRESS + (sEepromMetadata.u16NextPushStatusSaveIndex * PUSH_STATUS_PACKET_SIZE);

    if (!sv_eeprom_write(u32WriteAddress, (const uint8_t *)pData, sizeof(Push_Status_Data_t)))
    {
        return false;
    }

    sEepromMetadata.u16NextPushStatusSaveIndex++;

    if (sEepromMetadata.u16NextPushStatusSaveIndex >= MAX_PUSH_STATUS_COUNT)
    {
        sEepromMetadata.u16NextPushStatusSaveIndex = 0;
    }

    if (sEepromMetadata.u16PushStatusCount < MAX_PUSH_STATUS_COUNT)
    {
        sEepromMetadata.u16PushStatusCount++;
    }

    app_storage_eeprom_metadata_save(&sEepromMetadata);

    return true;
}

bool app_storage_push_status_load_latest(uint16_t u16Index, Push_Status_Data_t *pData)
{
    uint16_t u16ReadIndex;
    uint32_t u32ReadAddress;

    if ((pData == NULL) || (u16Index >= sEepromMetadata.u16PushStatusCount))
    {
        return false;
    }

    u16ReadIndex = (sEepromMetadata.u16NextPushStatusSaveIndex + MAX_PUSH_STATUS_COUNT - 1U - u16Index) % MAX_PUSH_STATUS_COUNT;

    u32ReadAddress = EEPROM_PUSH_STATUS_ADDRESS + (u16ReadIndex * PUSH_STATUS_PACKET_SIZE);

    return sv_eeprom_read(u32ReadAddress, (uint8_t *)pData, sizeof(Push_Status_Data_t));
}

void app_storage_push_status_clear(void)
{
    sEepromMetadata.u16PushStatusCount = 0;

    app_storage_eeprom_metadata_save(&sEepromMetadata);
}

bool app_storage_latch_save(const Latch_Data_t *pLatch)
{
    if (pLatch == NULL)
    {
        return false;
    }

    uint32_t u32WriteAddress = EEPROM_LATCH_ADDRESS + (sEepromMetadata.u16NextLatchSaveIndex * LATCH_PACKET_SIZE);

    if (!sv_eeprom_write(u32WriteAddress, (const uint8_t *)pLatch, sizeof(Latch_Data_t)))
    {
        return false;
    }

    sEepromMetadata.u16NextLatchSaveIndex++;

    if (sEepromMetadata.u16NextLatchSaveIndex >= MAX_LATCH_COUNT)
    {
        sEepromMetadata.u16NextLatchSaveIndex = 0;
    }

    if (sEepromMetadata.u16NextLatchSaveIndex == sEepromMetadata.u16NextLatchLoadIndex)
    {
        sEepromMetadata.u16NextLatchLoadIndex++;
        if (sEepromMetadata.u16NextLatchLoadIndex >= MAX_LATCH_COUNT)
        {
            sEepromMetadata.u16NextLatchLoadIndex = 0;
        }
    }

    if (sEepromMetadata.u16LatchCount < MAX_LATCH_COUNT)
    {
        sEepromMetadata.u16LatchCount++;
    }

    app_storage_eeprom_metadata_save(&sEepromMetadata);

    return true;
}

bool app_storage_latch_load_next(uint16_t u16LatchIndex, Latch_Data_t *pLatch)
{
    if ((pLatch == NULL) || (u16LatchIndex >= sEepromMetadata.u16LatchCount))
    {
        return false;
    }

    uint16_t u16LatchSendIndex = (sEepromMetadata.u16NextLatchLoadIndex + u16LatchIndex) % MAX_LATCH_COUNT;

    if (u16LatchSendIndex == sEepromMetadata.u16NextLatchSaveIndex)
    {
        return false; // No new data available
    }

    uint32_t u32ReadAddress = EEPROM_LATCH_ADDRESS + (u16LatchSendIndex * LATCH_PACKET_SIZE);
    return sv_eeprom_read(u32ReadAddress, (uint8_t *)pLatch, sizeof(Latch_Data_t));
}

bool app_storage_latch_load_latest(uint16_t u16LatchIndex, Latch_Data_t *pLatch)
{
    uint16_t u16LatchReadIndex;
    uint32_t u32ReadAddress;

    if ((pLatch == NULL) || (u16LatchIndex >= sEepromMetadata.u16LatchCount))
    {
        return false;
    }

    u16LatchReadIndex = (sEepromMetadata.u16NextLatchSaveIndex + MAX_LATCH_COUNT - 1U - u16LatchIndex) % MAX_LATCH_COUNT;

    u32ReadAddress = EEPROM_LATCH_ADDRESS + (u16LatchReadIndex * LATCH_PACKET_SIZE);

    return sv_eeprom_read(u32ReadAddress, (uint8_t *)pLatch, sizeof(Latch_Data_t));
}

void app_storage_latch_clear(void)
{
    sEepromMetadata.u16NextLatchLoadIndex = sEepromMetadata.u16NextLatchSaveIndex;
    sEepromMetadata.u16LatchCount = 0;

    app_storage_eeprom_metadata_save(&sEepromMetadata);
}

void app_storage_update_load_index(uint16_t u16LatchCount, uint16_t u16EventCount)
{
    sEepromMetadata.u16NextLatchLoadIndex = (sEepromMetadata.u16NextLatchLoadIndex + u16LatchCount) % MAX_LATCH_COUNT;
    sEepromMetadata.u16NextEventLoadIndex = (sEepromMetadata.u16NextEventLoadIndex + u16EventCount) % MAX_EVENT_COUNT;
    app_storage_eeprom_metadata_save(&sEepromMetadata);
}

void app_storage_runtime_data_update(void)
{
    Pulse_Count_t sPulseCount[MAX_PULSE_GATE_COUNT];
    for (uint8_t i = 0; i < MAX_PULSE_GATE_COUNT; i++)
    {
        sv_pulse_meter_get_count(i, &sPulseCount[i]);
    }
    memcpy(&sEepromRuntimeData.sPulseCount, &sPulseCount, sizeof(sPulseCount));
    app_storage_eeprom_runtime_data_save();
}

void app_storage_increase_reset_count(void)
{
    sEepromRuntimeData.u8ResetCount++;
    app_storage_eeprom_runtime_data_save();
}

void app_storage_set_reset_count(uint8_t u8Count)
{
    sEepromRuntimeData.u8ResetCount = u8Count;
    app_storage_eeprom_runtime_data_save();
}

uint8_t app_storage_get_reset_count(void)
{
    return sEepromRuntimeData.u8ResetCount;
}

void app_storage_get_metadata_runtime(Eeprom_Metadata_t *pMeta, Eeprom_Runtime_Data_t *pRuntime)
{
    memcpy(pMeta, &sEepromMetadata, sizeof(Eeprom_Metadata_t));
    memcpy(pRuntime, &sEepromRuntimeData, sizeof(Eeprom_Runtime_Data_t));
}

bool app_storage_set_pulse_meter_count(uint8_t u8MeterIndex, const Pulse_Meter_Data_t *pData)
{
    if (u8MeterIndex >= MAX_PULSE_GATE_COUNT)
    {
        return false;
    }

    bConfigChanged = true;

    Pulse_Count_t sPulseCount;
    sPulseCount.u64ForwardPulseCount = (uint64_t)(pData->dTotalForward * sPulseMeterConfig[u8MeterIndex].sConfig.u16PulseFactor + 0.5);
    sPulseCount.u64ReversePulseCount = (uint64_t)(pData->dTotalReverse * sPulseMeterConfig[u8MeterIndex].sConfig.u16PulseFactor + 0.5);

    memcpy(&sEepromRuntimeData.sPulseCount[u8MeterIndex], &sPulseCount, sizeof(Pulse_Count_t));
    app_storage_eeprom_runtime_data_save();

    sv_pulse_meter_set_count(u8MeterIndex, &sPulseCount);

    return true;
}

bool app_storage_set_module_config(const Module_Config_Parameter_t *pConfig)
{
    if (pConfig == NULL)
    {
        return false;
    }

    bConfigChanged = true;
    memcpy(&sModuleConfig, pConfig, sizeof(Module_Config_Parameter_t));
    return sv_eeprom_write(EEPROM_MODULE_CONFIG_ADDRESS, (const uint8_t *)&sModuleConfig, sizeof(sModuleConfig));
}

bool app_storage_get_module_config(Module_Config_Parameter_t *pConfig)
{
    if (pConfig == NULL)
    {
        return false;
    }

    memcpy(pConfig, &sModuleConfig, sizeof(Module_Config_Parameter_t));
    return true;
}

bool app_storage_set_charge_config(const Charge_Config_Parameter_t *pConfig)
{
    if (pConfig == NULL)
    {
        return false;
    }

    bConfigChanged = true;
    memcpy(&sChargeConfig, pConfig, sizeof(Charge_Config_Parameter_t));
    sv_charge_set_input_current_limit(sChargeConfig.u16IinLimMa);
    sv_charge_set_charge_voltage(sChargeConfig.u16ChargeVoltageMv);
    sv_charge_set_fast_charge_current(sChargeConfig.u16ChargeCurrentMa);
    if (sChargeConfig.u8ChargeLedEnable)
    {
        sv_charge_turn_on_led_status();
    }
    else
    {
        sv_charge_turn_off_led_status();
    }
    return sv_eeprom_write(EEPROM_CHARGE_CONFIG_ADDRESS, (const uint8_t *)&sChargeConfig, sizeof(sChargeConfig));
}

bool app_storage_get_charge_config(Charge_Config_Parameter_t *pConfig)
{
    if (pConfig == NULL)
    {
        return false;
    }

    memcpy(pConfig, &sChargeConfig, sizeof(Charge_Config_Parameter_t));
    return true;
}

bool app_storage_set_pulse_meter_config(uint8_t u8MeterIndex, const Pulse_Meter_Config_Parameter_t *pConfig)
{
    if ((u8MeterIndex >= MAX_PULSE_METER_COUNT) || (pConfig == NULL))
    {
        return false;
    }

    bConfigChanged = true;
    memcpy(&sPulseMeterConfig[u8MeterIndex], pConfig, sizeof(Pulse_Meter_Config_Parameter_t));
    sv_pulse_meter_set_config(u8MeterIndex, &pConfig->sConfig);
    return sv_eeprom_write(EEPROM_PULSE_METER_CONFIG_ADDRESS, (const uint8_t *)sPulseMeterConfig, sizeof(sPulseMeterConfig));
}

bool app_storage_get_pulse_meter_config(uint8_t u8MeterIndex, Pulse_Meter_Config_Parameter_t *pConfig)
{
    if ((u8MeterIndex >= MAX_PULSE_METER_COUNT) || (pConfig == NULL))
    {
        return false;
    }

    memcpy(pConfig, &sPulseMeterConfig[u8MeterIndex], sizeof(Pulse_Meter_Config_Parameter_t));
    return true;
}

bool app_storage_set_modbus_meter_config(uint8_t u8MeterIndex, const Modbus_Meter_Config_Parameter_t *pConfig)
{
    if ((u8MeterIndex >= MAX_MODBUS_METER_COUNT) || (pConfig == NULL))
    {
        return false;
    }

    bConfigChanged = true;
    memcpy(&sModbusMeterConfig[u8MeterIndex], pConfig, sizeof(Modbus_Meter_Config_Parameter_t));
    return sv_eeprom_write(EEPROM_MODBUS_METER_CONFIG_ADDRESS, (const uint8_t *)sModbusMeterConfig, sizeof(sModbusMeterConfig));
}

bool app_storage_get_modbus_meter_config(uint8_t u8MeterIndex, Modbus_Meter_Config_Parameter_t *pConfig)
{
    if ((u8MeterIndex >= MAX_MODBUS_METER_COUNT) || (pConfig == NULL))
    {
        return false;
    }

    memcpy(pConfig, &sModbusMeterConfig[u8MeterIndex], sizeof(Modbus_Meter_Config_Parameter_t));
    return true;
}

bool app_storage_set_pressure_sensor_config(uint8_t u8MeterIndex, const Pressure_Sensor_Config_Parameter_t *pConfig)
{
    if ((u8MeterIndex >= MAX_PRESSURE_SENSOR_COUNT) || (pConfig == NULL))
    {
        return false;
    }

    bConfigChanged = true;
    memcpy(&sPressureSensorConfig[u8MeterIndex], pConfig, sizeof(Pressure_Sensor_Config_Parameter_t));
    return sv_eeprom_write(EEPROM_PRESSURE_SENSOR_CONFIG_ADDRESS, (const uint8_t *)sPressureSensorConfig, sizeof(sPressureSensorConfig));
}

bool app_storage_get_pressure_sensor_config(uint8_t u8MeterIndex, Pressure_Sensor_Config_Parameter_t *pConfig)
{
    if ((u8MeterIndex >= MAX_PRESSURE_SENSOR_COUNT) || (pConfig == NULL))
    {
        return false;
    }

    memcpy(pConfig, &sPressureSensorConfig[u8MeterIndex], sizeof(Pressure_Sensor_Config_Parameter_t));
    return true;
}

void app_storage_config_parameter_default(void)
{
    memset(&sIpEndpoint, 0, sizeof(sIpEndpoint));
    memcpy(sIpEndpoint.au8Ipv4, IPV4_ADDRESS, strlen(IPV4_ADDRESS));
    memcpy(sIpEndpoint.au8Port, IPV4_PORT, strlen(IPV4_PORT));
    sv_eeprom_write(EEPROM_IP_ENDPOINT_ADDRESS, (const uint8_t *)&sIpEndpoint, sizeof(sIpEndpoint));

    sModuleConfig.u16LatchPeriod = LATCH_PERIOD;
    sModuleConfig.u16PushPeriod = PUSH_PERIOD;
    sModuleConfig.fTimezone = TIMEZONE;
    sv_eeprom_write(EEPROM_MODULE_CONFIG_ADDRESS, (const uint8_t *)&sModuleConfig, sizeof(sModuleConfig));

    sChargeConfig.u16IinLimMa = CHARGE_IINLIM_MA;
    sChargeConfig.u16ChargeVoltageMv = CHARGE_VOLTAGE_MV;
    sChargeConfig.u16ChargeCurrentMa = CHARGE_CURRENT_MA;
    sChargeConfig.u8ChargeLedEnable = CHARGE_LED_ENABLE;
    sv_eeprom_write(EEPROM_CHARGE_CONFIG_ADDRESS, (const uint8_t *)&sChargeConfig, sizeof(sChargeConfig));

    for (uint8_t i = 0; i < MAX_PULSE_METER_COUNT; i++)
    {
        sPulseMeterConfig[i].u8MeterEnable = 0;

        memset(sPulseMeterConfig[i].u8Serial, 0, sizeof(sPulseMeterConfig[i].u8Serial));

        sPulseMeterConfig[i].sConfig.u16PulseFactor = PULSE_FACTOR;
        sPulseMeterConfig[i].sConfig.sPulseConfig.u8Pin1Select = PULSE_PIN_1;
        sPulseMeterConfig[i].sConfig.sPulseConfig.u8Pin2Select = PULSE_PIN_2;
        sPulseMeterConfig[i].sConfig.sPulseConfig.u8EdgeType = PULSE_EDGE_TYPE;
        sPulseMeterConfig[i].sConfig.sPulseConfig.u8PulseType = PULSE_TYPE;
    }
    sv_eeprom_write(EEPROM_PULSE_METER_CONFIG_ADDRESS, (const uint8_t *)sPulseMeterConfig, sizeof(sPulseMeterConfig));

    for (uint8_t i = 0; i < MAX_MODBUS_METER_COUNT; i++)
    {
        sModbusMeterConfig[i].u8MeterEnable = 0;

        memset(sModbusMeterConfig[i].u8Serial, 0, sizeof(sModbusMeterConfig[i].u8Serial));

        sModbusMeterConfig[i].sConfig.sModbusConfig.u8SlaveAddress = MODBUS_SLAVE_ADDRESS;
        sModbusMeterConfig[i].sConfig.sModbusConfig.u32BaudRate = MODBUS_BAUDRATE;
        sModbusMeterConfig[i].sConfig.sModbusConfig.u8SerialConfig = MODBUS_SERIAL;
        sModbusMeterConfig[i].sConfig.sModbusConfig.u8ReadFuncCode = MODBUS_READ_FUNCTION_CODE;

        sModbusMeterConfig[i].sConfig.sForwardTotalizer.u8ParameterEnable = FORWARD_TOTALIZER_ENABLE;
        sModbusMeterConfig[i].sConfig.sForwardTotalizer.u16RegisterAddress = FORWARD_TOTALIZER_ADDRESS;
        sModbusMeterConfig[i].sConfig.sForwardTotalizer.u8DataType = FORWARD_TOTALIZER_DATA_TYPE;
        sModbusMeterConfig[i].sConfig.sForwardTotalizer.u8WordSwap = FORWARD_TOTALIZER_WORD_SWAP;
        sModbusMeterConfig[i].sConfig.sForwardTotalizer.s8Multiplier = FORWARD_TOTALIZER_MULTIPLIER;

        sModbusMeterConfig[i].sConfig.sReverseTotalizer.u8ParameterEnable = REVERSE_TOTALIZER_ENABLE;
        sModbusMeterConfig[i].sConfig.sReverseTotalizer.u16RegisterAddress = REVERSE_TOTALIZER_ADDRESS;
        sModbusMeterConfig[i].sConfig.sReverseTotalizer.u8DataType = REVERSE_TOTALIZER_DATA_TYPE;
        sModbusMeterConfig[i].sConfig.sReverseTotalizer.u8WordSwap = REVERSE_TOTALIZER_WORD_SWAP;
        sModbusMeterConfig[i].sConfig.sReverseTotalizer.s8Multiplier = REVERSE_TOTALIZER_MULTIPLIER;

        sModbusMeterConfig[i].sConfig.sFlowRate.u8ParameterEnable = FLOW_RATE_ENABLE;
        sModbusMeterConfig[i].sConfig.sFlowRate.u16RegisterAddress = FLOW_RATE_ADDRESS;
        sModbusMeterConfig[i].sConfig.sFlowRate.u8DataType = FLOW_RATE_DATA_TYPE;
        sModbusMeterConfig[i].sConfig.sFlowRate.u8WordSwap = FLOW_RATE_WORD_SWAP;
        sModbusMeterConfig[i].sConfig.sFlowRate.s8Multiplier = FLOW_RATE_MULTIPLIER;
    }
    sv_eeprom_write(EEPROM_MODBUS_METER_CONFIG_ADDRESS, (const uint8_t *)sModbusMeterConfig, sizeof(sModbusMeterConfig));

    for (uint8_t i = 0; i < MAX_PRESSURE_SENSOR_COUNT; i++)
    {
        sPressureSensorConfig[i].u8MeterEnable = 0;

        memset(sPressureSensorConfig[i].u8Serial, 0, sizeof(sPressureSensorConfig[i].u8Serial));

        sPressureSensorConfig[i].sConfig.fMinCurrent = SENSOR_MIN_CURRENT;
        sPressureSensorConfig[i].sConfig.fMaxCurrent = SENSOR_MAX_CURRENT;
        sPressureSensorConfig[i].sConfig.fMinPressure = SENSOR_MIN_PRESSURE;
        sPressureSensorConfig[i].sConfig.fMaxPressure = SENSOR_MAX_PRESSURE;
    }
    sv_eeprom_write(EEPROM_PRESSURE_SENSOR_CONFIG_ADDRESS, (const uint8_t *)sPressureSensorConfig, sizeof(sPressureSensorConfig));
}

uint8_t app_storage_get_meter_serial(Meter_Type_t eMeterType, uint8_t u8MeterIndex, uint8_t *pSerial)
{
    if (pSerial == NULL)
    {
        return 0;
    }

    switch (eMeterType)
    {
    case MODULE_TYPE:
    {
        uint64_t u64Serial = u64ModuleSerial;
        uint8_t au8ModuleSerial[MODULE_SERIAL_SIZE];
        for (int8_t i = MODULE_SERIAL_SIZE - 1; i >= 0; i--)
        {
            au8ModuleSerial[i] = '0' + (u64Serial % 10);
            u64Serial /= 10;
        }
        memcpy(pSerial, au8ModuleSerial, MODULE_SERIAL_SIZE);
        return MODULE_SERIAL_SIZE;
    }

    case PULSE_METER_TYPE:
        if (u8MeterIndex >= MAX_PULSE_METER_COUNT)
        {
            return 0;
        }
        memcpy(pSerial, sPulseMeterConfig[u8MeterIndex].u8Serial, METER_SERIAL_SIZE);
        if (sPulseMeterConfig[u8MeterIndex].u8Serial[METER_SERIAL_SIZE - 1] != '\0')
        {
            return METER_SERIAL_SIZE;
        }
        else
        {
            return strlen((const char *)sPulseMeterConfig[u8MeterIndex].u8Serial);
        }

    case MODBUS_METER_TYPE:
        if (u8MeterIndex >= MAX_MODBUS_METER_COUNT)
        {
            return 0;
        }
        memcpy(pSerial, sModbusMeterConfig[u8MeterIndex].u8Serial, METER_SERIAL_SIZE);
        if (sModbusMeterConfig[u8MeterIndex].u8Serial[METER_SERIAL_SIZE - 1] != '\0')
        {
            return METER_SERIAL_SIZE;
        }
        else
        {
            return strlen((const char *)sModbusMeterConfig[u8MeterIndex].u8Serial);
        }

    case PRESSURE_SENSOR_TYPE:
        if (u8MeterIndex >= MAX_PRESSURE_SENSOR_COUNT)
        {
            return 0;
        }
        memcpy(pSerial, sPressureSensorConfig[u8MeterIndex].u8Serial, METER_SERIAL_SIZE);
        if (sPressureSensorConfig[u8MeterIndex].u8Serial[METER_SERIAL_SIZE - 1] != '\0')
        {
            return METER_SERIAL_SIZE;
        }
        else
        {
            return strlen((const char *)sPressureSensorConfig[u8MeterIndex].u8Serial);
        }

    default:
        return 0;
    }
}

bool app_storage_check_meter_active(Meter_Type_t eMeterType, uint8_t u8MeterIndex)
{
    switch (eMeterType)
    {
    case MODULE_TYPE:
        return true;

    case PULSE_METER_TYPE:
        if (u8MeterIndex >= MAX_PULSE_METER_COUNT)
        {
            return false;
        }
        return sPulseMeterConfig[u8MeterIndex].u8MeterEnable;

    case MODBUS_METER_TYPE:
        if (u8MeterIndex >= MAX_MODBUS_METER_COUNT)
        {
            return false;
        }
        return sModbusMeterConfig[u8MeterIndex].u8MeterEnable;

    case PRESSURE_SENSOR_TYPE:
        if (u8MeterIndex >= MAX_PRESSURE_SENSOR_COUNT)
        {
            return false;
        }
        return sPressureSensorConfig[u8MeterIndex].u8MeterEnable;

    default:
        return false;
    }
}

/*==================================================================================================
*                                   PRIVATE FUNCTIONS DEFINITIONS
==================================================================================================*/

static void app_storage_eeprom_header_init(void)
{
    Eeprom_Header_t sEepromHeader = {
        .u32Magic = EEPROM_MAGIC_NUMBER,
        .au8Version = EEPROM_LAYOUT_VERSION,
    };

    sv_eeprom_write(EEPROM_HEADER_ADDRESS, (const uint8_t *)&sEepromHeader, sizeof(Eeprom_Header_t));
}

static void app_storage_eeprom_metadata_init(void)
{
    memset(&sEepromMetadata, 0, sizeof(sEepromMetadata));

    sv_eeprom_write(EEPROM_METADATA_AREA_1, (const uint8_t *)&sEepromMetadata, sizeof(sEepromMetadata));
    sv_eeprom_write(EEPROM_METADATA_AREA_2, (const uint8_t *)&sEepromMetadata, sizeof(sEepromMetadata));
    sv_eeprom_write(EEPROM_METADATA_AREA_3, (const uint8_t *)&sEepromMetadata, sizeof(sEepromMetadata));
    sv_eeprom_write(EEPROM_METADATA_AREA_4, (const uint8_t *)&sEepromMetadata, sizeof(sEepromMetadata));
}

static void app_storage_eeprom_metadata_load(void)
{
    Eeprom_Metadata_t sMeta1, sMeta2, sMeta3, sMeta4;

    if (!sv_eeprom_read(EEPROM_METADATA_AREA_1, (uint8_t *)&sMeta1, sizeof(sMeta1)))
    {
        memset(&sMeta1, 0, sizeof(sMeta1));
    }

    if (!sv_eeprom_read(EEPROM_METADATA_AREA_2, (uint8_t *)&sMeta2, sizeof(sMeta2)))
    {
        memset(&sMeta2, 0, sizeof(sMeta2));
    }

    if (!sv_eeprom_read(EEPROM_METADATA_AREA_3, (uint8_t *)&sMeta3, sizeof(sMeta3)))
    {
        memset(&sMeta3, 0, sizeof(sMeta3));
    }

    if (!sv_eeprom_read(EEPROM_METADATA_AREA_4, (uint8_t *)&sMeta4, sizeof(sMeta4)))
    {
        memset(&sMeta4, 0, sizeof(sMeta4));
    }

    uint32_t u32LastSequence = 0;
    if (sMeta1.u32Sequence >= u32LastSequence)
    {
        u32LastSequence = sMeta1.u32Sequence;
        memcpy(&sEepromMetadata, &sMeta1, sizeof(Eeprom_Metadata_t));
        eNextEepromMetadataLocation = EEPROM_METADATA_AREA_2; // Latest record meta data at location 1, next record meta data at location 2
    }
    if (sMeta2.u32Sequence > u32LastSequence)
    {
        u32LastSequence = sMeta2.u32Sequence;
        memcpy(&sEepromMetadata, &sMeta2, sizeof(Eeprom_Metadata_t));
        eNextEepromMetadataLocation = EEPROM_METADATA_AREA_3;
    }
    if (sMeta3.u32Sequence > u32LastSequence)
    {
        u32LastSequence = sMeta3.u32Sequence;
        memcpy(&sEepromMetadata, &sMeta3, sizeof(Eeprom_Metadata_t));
        eNextEepromMetadataLocation = EEPROM_METADATA_AREA_4;
    }
    if (sMeta4.u32Sequence > u32LastSequence)
    {
        memcpy(&sEepromMetadata, &sMeta4, sizeof(Eeprom_Metadata_t));
        eNextEepromMetadataLocation = EEPROM_METADATA_AREA_1;
    }
}

static void app_storage_eeprom_metadata_save(void)
{
    sEepromMetadata.u32Sequence++;
    Eeprom_Metadata_Area_t eLocation = eNextEepromMetadataLocation;

    sv_eeprom_write(eLocation, (const uint8_t *)&sEepromMetadata, sizeof(sEepromMetadata));

    switch (eLocation)
    {
    case EEPROM_METADATA_AREA_1:
        eNextEepromMetadataLocation = EEPROM_METADATA_AREA_2;
        break;

    case EEPROM_METADATA_AREA_2:
        eNextEepromMetadataLocation = EEPROM_METADATA_AREA_3;
        break;

    case EEPROM_METADATA_AREA_3:
        eNextEepromMetadataLocation = EEPROM_METADATA_AREA_4;
        break;

    case EEPROM_METADATA_AREA_4:
        eNextEepromMetadataLocation = EEPROM_METADATA_AREA_1;
        break;

    default:
        eNextEepromMetadataLocation = EEPROM_METADATA_AREA_1;
        break;
    }
}

static void app_storage_eeprom_runtime_data_init(void)
{
    memset(&sEepromRuntimeData, 0, sizeof(sEepromRuntimeData));

    sv_eeprom_write(EEPROM_RUNTIME_DATA_AREA_1, (const uint8_t *)&sEepromRuntimeData, sizeof(sEepromRuntimeData));
    sv_eeprom_write(EEPROM_RUNTIME_DATA_AREA_2, (const uint8_t *)&sEepromRuntimeData, sizeof(sEepromRuntimeData));
    sv_eeprom_write(EEPROM_RUNTIME_DATA_AREA_3, (const uint8_t *)&sEepromRuntimeData, sizeof(sEepromRuntimeData));
    sv_eeprom_write(EEPROM_RUNTIME_DATA_AREA_4, (const uint8_t *)&sEepromRuntimeData, sizeof(sEepromRuntimeData));
}

static void app_storage_eeprom_runtime_data_load(void)
{
    Eeprom_Runtime_Data_t sRuntime1, sRuntime2, sRuntime3, sRuntime4;

    if (!sv_eeprom_read(EEPROM_RUNTIME_DATA_AREA_1, (uint8_t *)&sRuntime1, sizeof(sRuntime1)))
    {
        memset(&sRuntime1, 0, sizeof(sRuntime1));
    }
    if (!sv_eeprom_read(EEPROM_RUNTIME_DATA_AREA_2, (uint8_t *)&sRuntime2, sizeof(sRuntime2)))
    {
        memset(&sRuntime2, 0, sizeof(sRuntime2));
    }
    if (!sv_eeprom_read(EEPROM_RUNTIME_DATA_AREA_3, (uint8_t *)&sRuntime3, sizeof(sRuntime3)))
    {
        memset(&sRuntime3, 0, sizeof(sRuntime3));
    }
    if (!sv_eeprom_read(EEPROM_RUNTIME_DATA_AREA_4, (uint8_t *)&sRuntime4, sizeof(sRuntime4)))
    {
        memset(&sRuntime4, 0, sizeof(sRuntime4));
    }

    uint32_t u32LastSequence = 0;
    if (sRuntime1.u32Sequence >= u32LastSequence)
    {
        u32LastSequence = sRuntime1.u32Sequence;
        memcpy(&sEepromRuntimeData, &sRuntime1, sizeof(sRuntime1));
        eNextEepromRuntimeDataLocation = EEPROM_RUNTIME_DATA_AREA_2; // Latest runtime data at location 1, next runtime data at location 2
    }
    if (sRuntime2.u32Sequence >= u32LastSequence)
    {
        u32LastSequence = sRuntime2.u32Sequence;
        memcpy(&sEepromRuntimeData, &sRuntime2, sizeof(sRuntime2));
        eNextEepromRuntimeDataLocation = EEPROM_RUNTIME_DATA_AREA_3;
    }
    if (sRuntime3.u32Sequence >= u32LastSequence)
    {
        u32LastSequence = sRuntime3.u32Sequence;
        memcpy(&sEepromRuntimeData, &sRuntime3, sizeof(sRuntime3));
        eNextEepromRuntimeDataLocation = EEPROM_RUNTIME_DATA_AREA_4;
    }
    if (sRuntime4.u32Sequence >= u32LastSequence)
    {
        memcpy(&sEepromRuntimeData, &sRuntime4, sizeof(sRuntime4));
        eNextEepromRuntimeDataLocation = EEPROM_RUNTIME_DATA_AREA_1;
    }
}

static void app_storage_eeprom_runtime_data_save(void)
{
    sEepromRuntimeData.u32Sequence++;
    Eeprom_Runtime_Data_Area_t eLocation = eNextEepromRuntimeDataLocation;

    sv_eeprom_write(eLocation, (const uint8_t *)&sEepromRuntimeData, sizeof(sEepromRuntimeData));

    switch (eLocation)
    {
    case EEPROM_RUNTIME_DATA_AREA_1:
        eNextEepromRuntimeDataLocation = EEPROM_RUNTIME_DATA_AREA_2;
        break;

    case EEPROM_RUNTIME_DATA_AREA_2:
        eNextEepromRuntimeDataLocation = EEPROM_RUNTIME_DATA_AREA_3;
        break;

    case EEPROM_RUNTIME_DATA_AREA_3:
        eNextEepromRuntimeDataLocation = EEPROM_RUNTIME_DATA_AREA_4;
        break;

    case EEPROM_RUNTIME_DATA_AREA_4:
        eNextEepromRuntimeDataLocation = EEPROM_RUNTIME_DATA_AREA_1;
        break;

    default:
        eNextEepromRuntimeDataLocation = EEPROM_RUNTIME_DATA_AREA_1;
        break;
    }
}

static void app_storage_module_serial_load(void)
{
    sv_eeprom_read(EEPROM_DEVICE_SERIAL_ADDRESS, (uint8_t *)&u64ModuleSerial, sizeof(u64ModuleSerial));
}

static void app_storage_password_load(void)
{
    sv_eeprom_read(EEPROM_DEVICE_PASSWORD_ADDRESS, (uint8_t *)&sDevicePassword, sizeof(sDevicePassword));
}

static void app_storage_config_parameter_load(void)
{
    sv_eeprom_read(EEPROM_IP_ENDPOINT_ADDRESS, (uint8_t *)&sIpEndpoint, sizeof(sIpEndpoint));
    sv_eeprom_read(EEPROM_MODULE_CONFIG_ADDRESS, (uint8_t *)&sModuleConfig, sizeof(sModuleConfig));
    sv_eeprom_read(EEPROM_CHARGE_CONFIG_ADDRESS, (uint8_t *)&sChargeConfig, sizeof(sChargeConfig));
    sv_eeprom_read(EEPROM_PULSE_METER_CONFIG_ADDRESS, (uint8_t *)sPulseMeterConfig, sizeof(sPulseMeterConfig));
    sv_eeprom_read(EEPROM_MODBUS_METER_CONFIG_ADDRESS, (uint8_t *)sModbusMeterConfig, sizeof(sModbusMeterConfig));
    sv_eeprom_read(EEPROM_PRESSURE_SENSOR_CONFIG_ADDRESS, (uint8_t *)sPressureSensorConfig, sizeof(sPressureSensorConfig));
}