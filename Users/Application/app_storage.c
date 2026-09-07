#include "app_storage.h"

static uint64_t u64ModuleSerial;

static Eeprom_Sequence_t u64EepromMetadataSequence;
static Eeprom_Metadata_t sEepromMetadata;
static Eeprom_Metadata_Area_t eNextEepromMetadataLocation;

static Eeprom_Sequence_t u64EepromRuntimeDataSequence;
static Eeprom_Runtime_Data_t sEepromRuntimeData;
static Eeprom_Runtime_Data_Area_t eNextEepromRuntimeDataLocation;

static Ip_Endpoint_t sIpEndpoint;
static Module_Config_Parameter_t sModuleConfig;
static Pulse_Meter_Config_Parameter_t sPulseMeterConfig[MAX_PULSE_METER_COUNT];
static Modbus_Meter_Config_Parameter_t sModbusMeterConfig[MAX_MODBUS_METER_COUNT];
static Pressure_Sensor_Config_Parameter_t sPressureSensorConfig[MAX_PRESSURE_SENSOR_COUNT];

static volatile const Pulse_Count_t *const pPulseCount = (volatile Pulse_Count_t *)RAM_NOINIT_PULSE_COUNT_ADDRESS;
static volatile const uint16_t *const pPulseCountCrc = (volatile uint16_t *)RAM_NOINIT_PULSE_COUNT_CRC_ADDRESS;

static uint8_t au8StorageBuf[STORAGE_BUFFER_SIZE];

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
 * @brief Initialize configuration parameters.
 */
static void app_storage_config_parameter_init(void);

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
        app_storage_config_parameter_init();

        app_storage_eeprom_header_init();
    }
    else
    {
        app_storage_eeprom_metadata_load();
        app_storage_eeprom_runtime_data_load();
        app_storage_config_parameter_load();
    }

    // Load module serial
    app_storage_module_serial_load();

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

void app_storage_get_ip_endpoint(Ip_Endpoint_t *pIpEndpoint)
{
    memcpy(pIpEndpoint, &sIpEndpoint, sizeof(Ip_Endpoint_t));
}

uint16_t app_storage_get_latch_period(void)
{
    return sModuleConfig.u16LatchPeriod;
}

uint16_t app_storage_get_record_period(void)
{
    return sModuleConfig.u16PushPeriod;
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

void app_storage_event_update_load_index(uint16_t u16EventCount)
{
    sEepromMetadata.u16NextEventLoadIndex = (sEepromMetadata.u16NextEventLoadIndex + u16EventCount) % MAX_EVENT_COUNT;
    app_storage_eeprom_metadata_save(&sEepromMetadata);
}

void app_storage_event_clear(void)
{
    sEepromMetadata.u16NextEventLoadIndex = sEepromMetadata.u16NextEventSaveIndex;
    sEepromMetadata.u16EventCount = 0;

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

void app_storage_latch_update_load_index(uint16_t u16LatchCount)
{
    sEepromMetadata.u16NextLatchLoadIndex = (sEepromMetadata.u16NextLatchLoadIndex + u16LatchCount) % MAX_LATCH_COUNT;
    app_storage_eeprom_metadata_save(&sEepromMetadata);
}

void app_storage_latch_clear(void)
{
    sEepromMetadata.u16NextLatchLoadIndex = sEepromMetadata.u16NextLatchSaveIndex;
    sEepromMetadata.u16LatchCount = 0;

    app_storage_eeprom_metadata_save(&sEepromMetadata);
}

bool app_storage_runtime_data_update(void)
{
    uint16_t u16Crc = *pPulseCountCrc;

    if ((sys_crc16((uint8_t *)pPulseCount, sizeof(Pulse_Count_t) * MAX_PULSE_GATE_COUNT) == u16Crc))
    {
        __disable_irq();
        memcpy(sEepromRuntimeData.sPulseCount, (const void *)pPulseCount, sizeof(sEepromRuntimeData.sPulseCount));
        __enable_irq();

        app_storage_eeprom_runtime_data_save();
        return true;
    }

    return false;
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
    u64EepromMetadataSequence = 0;
    memset(&sEepromMetadata, 0, sizeof(Eeprom_Metadata_t));

    uint32_t u32WriteLen = sizeof(Eeprom_Sequence_t) + sizeof(Eeprom_Metadata_t);
    memcpy(&au8StorageBuf[0], &u64EepromMetadataSequence, sizeof(Eeprom_Sequence_t));
    memcpy(&au8StorageBuf[sizeof(Eeprom_Sequence_t)], &sEepromMetadata, sizeof(Eeprom_Metadata_t));

    sv_eeprom_write(EEPROM_METADATA_AREA_1, au8StorageBuf, u32WriteLen);
    sv_eeprom_write(EEPROM_METADATA_AREA_2, au8StorageBuf, u32WriteLen);
    sv_eeprom_write(EEPROM_METADATA_AREA_3, au8StorageBuf, u32WriteLen);
    sv_eeprom_write(EEPROM_METADATA_AREA_4, au8StorageBuf, u32WriteLen);
}

static void app_storage_eeprom_metadata_load(void)
{
    Eeprom_Metadata_t sMeta1, sMeta2, sMeta3, sMeta4;
    Eeprom_Sequence_t u64Sequence1, u64Sequence2, u64Sequence3, u64Sequence4;
    uint32_t u32MetadataLen = sizeof(Eeprom_Sequence_t) + sizeof(Eeprom_Metadata_t);

    if (sv_eeprom_read(EEPROM_METADATA_AREA_1, au8StorageBuf, u32MetadataLen))
    {
        memcpy(&u64Sequence1, &au8StorageBuf[0], sizeof(Eeprom_Sequence_t));
        memcpy(&sMeta1, &au8StorageBuf[sizeof(Eeprom_Sequence_t)], sizeof(Eeprom_Metadata_t));
    }
    else
    {
        u64Sequence1 = 0;
        memset(&sMeta1, 0, sizeof(Eeprom_Metadata_t));
    }

    if (sv_eeprom_read(EEPROM_METADATA_AREA_2, au8StorageBuf, u32MetadataLen))
    {
        memcpy(&u64Sequence2, &au8StorageBuf[0], sizeof(Eeprom_Sequence_t));
        memcpy(&sMeta2, &au8StorageBuf[sizeof(Eeprom_Sequence_t)], sizeof(Eeprom_Metadata_t));
    }
    else
    {
        u64Sequence2 = 0;
        memset(&sMeta2, 0, sizeof(Eeprom_Metadata_t));
    }

    if (sv_eeprom_read(EEPROM_METADATA_AREA_3, au8StorageBuf, u32MetadataLen))
    {
        memcpy(&u64Sequence3, &au8StorageBuf[0], sizeof(Eeprom_Sequence_t));
        memcpy(&sMeta3, &au8StorageBuf[sizeof(Eeprom_Sequence_t)], sizeof(Eeprom_Metadata_t));
    }
    else
    {
        u64Sequence3 = 0;
        memset(&sMeta3, 0, sizeof(Eeprom_Metadata_t));
    }

    if (sv_eeprom_read(EEPROM_METADATA_AREA_4, au8StorageBuf, u32MetadataLen))
    {
        memcpy(&u64Sequence4, &au8StorageBuf[0], sizeof(Eeprom_Sequence_t));
        memcpy(&sMeta4, &au8StorageBuf[sizeof(Eeprom_Sequence_t)], sizeof(Eeprom_Metadata_t));
    }
    else
    {
        u64Sequence4 = 0;
        memset(&sMeta4, 0, sizeof(Eeprom_Metadata_t));
    }

    Eeprom_Sequence_t LastSequence = 0;
    if (u64Sequence1 >= LastSequence)
    {
        memcpy(&sEepromMetadata, &sMeta1, sizeof(Eeprom_Metadata_t));
        LastSequence = u64Sequence1;
        eNextEepromMetadataLocation = EEPROM_METADATA_AREA_2; // Latest record meta data at location 1, next record meta data at location 2
    }
    if (u64Sequence2 > LastSequence)
    {
        memcpy(&sEepromMetadata, &sMeta2, sizeof(Eeprom_Metadata_t));
        LastSequence = u64Sequence2;
        eNextEepromMetadataLocation = EEPROM_METADATA_AREA_3;
    }
    if (u64Sequence3 > LastSequence)
    {
        memcpy(&sEepromMetadata, &sMeta3, sizeof(Eeprom_Metadata_t));
        LastSequence = u64Sequence3;
        eNextEepromMetadataLocation = EEPROM_METADATA_AREA_4;
    }
    if (u64Sequence4 > LastSequence)
    {
        memcpy(&sEepromMetadata, &sMeta4, sizeof(Eeprom_Metadata_t));
        LastSequence = u64Sequence4;
        eNextEepromMetadataLocation = EEPROM_METADATA_AREA_1;
    }

    u64EepromMetadataSequence = LastSequence;
}

static void app_storage_eeprom_metadata_save(void)
{
    u64EepromMetadataSequence++;
    Eeprom_Metadata_Area_t eLocation = eNextEepromMetadataLocation;

    uint32_t u32MetadataLen = sizeof(Eeprom_Sequence_t) + sizeof(Eeprom_Metadata_t);
    memcpy(&au8StorageBuf[0], &u64EepromMetadataSequence, sizeof(Eeprom_Sequence_t));
    memcpy(&au8StorageBuf[sizeof(Eeprom_Sequence_t)], &sEepromMetadata, sizeof(Eeprom_Metadata_t));
    sv_eeprom_write(eLocation, au8StorageBuf, u32MetadataLen);

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
    }
}

static void app_storage_eeprom_runtime_data_init(void)
{
    Eeprom_Sequence_t Sequence = 0;
    Eeprom_Runtime_Data_t Runtime = {0};

    uint32_t u32WriteLen = sizeof(Eeprom_Sequence_t) + sizeof(Eeprom_Runtime_Data_t);
    memcpy(&au8StorageBuf[0], &Sequence, sizeof(Eeprom_Sequence_t));
    memcpy(&au8StorageBuf[sizeof(Eeprom_Sequence_t)], &Runtime, sizeof(Eeprom_Runtime_Data_t));

    sv_eeprom_write(EEPROM_RUNTIME_DATA_AREA_1, au8StorageBuf, u32WriteLen);
    sv_eeprom_write(EEPROM_RUNTIME_DATA_AREA_2, au8StorageBuf, u32WriteLen);
    sv_eeprom_write(EEPROM_RUNTIME_DATA_AREA_3, au8StorageBuf, u32WriteLen);
    sv_eeprom_write(EEPROM_RUNTIME_DATA_AREA_4, au8StorageBuf, u32WriteLen);
}

static void app_storage_eeprom_runtime_data_load(void)
{
    Eeprom_Runtime_Data_t sRuntime1, sRuntime2, sRuntime3, sRuntime4;
    Eeprom_Sequence_t u64Sequence1, u64Sequence2, u64Sequence3, u64Sequence4;
    uint32_t u32RuntimeDataLen = sizeof(Eeprom_Sequence_t) + sizeof(Eeprom_Runtime_Data_t);

    if (sv_eeprom_read(EEPROM_RUNTIME_DATA_AREA_1, au8StorageBuf, u32RuntimeDataLen))
    {
        memcpy(&u64Sequence1, &au8StorageBuf[0], sizeof(Eeprom_Sequence_t));
        memcpy(&sRuntime1, &au8StorageBuf[sizeof(Eeprom_Sequence_t)], sizeof(Eeprom_Runtime_Data_t));
    }
    else
    {
        u64Sequence1 = 0;
        memset(&sRuntime1, 0, sizeof(Eeprom_Runtime_Data_t));
    }

    if (sv_eeprom_read(EEPROM_RUNTIME_DATA_AREA_2, au8StorageBuf, u32RuntimeDataLen))
    {
        memcpy(&u64Sequence2, &au8StorageBuf[0], sizeof(Eeprom_Sequence_t));
        memcpy(&sRuntime2, &au8StorageBuf[sizeof(Eeprom_Sequence_t)], sizeof(Eeprom_Runtime_Data_t));
    }
    else
    {
        u64Sequence2 = 0;
        memset(&sRuntime2, 0, sizeof(Eeprom_Runtime_Data_t));
    }

    if (sv_eeprom_read(EEPROM_RUNTIME_DATA_AREA_3, au8StorageBuf, u32RuntimeDataLen))
    {
        memcpy(&u64Sequence3, &au8StorageBuf[0], sizeof(Eeprom_Sequence_t));
        memcpy(&sRuntime3, &au8StorageBuf[sizeof(Eeprom_Sequence_t)], sizeof(Eeprom_Runtime_Data_t));
    }
    else
    {
        u64Sequence3 = 0;
        memset(&sRuntime3, 0, sizeof(Eeprom_Runtime_Data_t));
    }

    if (sv_eeprom_read(EEPROM_RUNTIME_DATA_AREA_4, au8StorageBuf, u32RuntimeDataLen))
    {
        memcpy(&u64Sequence4, &au8StorageBuf[0], sizeof(Eeprom_Sequence_t));
        memcpy(&sRuntime4, &au8StorageBuf[sizeof(Eeprom_Sequence_t)], sizeof(Eeprom_Runtime_Data_t));
    }
    else
    {
        u64Sequence4 = 0;
        memset(&sRuntime4, 0, sizeof(Eeprom_Runtime_Data_t));
    }

    Eeprom_Sequence_t LastSequence = 0;
    if (u64Sequence1 >= LastSequence)
    {
        memcpy(&sEepromRuntimeData, &sRuntime1, sizeof(Eeprom_Metadata_t));
        LastSequence = u64Sequence1;
        eNextEepromRuntimeDataLocation = EEPROM_RUNTIME_DATA_AREA_2; // Latest runtime data at location 1, next runtime data at location 2
    }
    if (u64Sequence2 > LastSequence)
    {
        memcpy(&sEepromRuntimeData, &sRuntime2, sizeof(Eeprom_Metadata_t));
        LastSequence = u64Sequence2;
        eNextEepromRuntimeDataLocation = EEPROM_RUNTIME_DATA_AREA_3;
    }
    if (u64Sequence3 > LastSequence)
    {
        memcpy(&sEepromRuntimeData, &sRuntime3, sizeof(Eeprom_Metadata_t));
        LastSequence = u64Sequence3;
        eNextEepromRuntimeDataLocation = EEPROM_RUNTIME_DATA_AREA_4;
    }
    if (u64Sequence4 > LastSequence)
    {
        memcpy(&sEepromRuntimeData, &sRuntime4, sizeof(Eeprom_Metadata_t));
        LastSequence = u64Sequence4;
        eNextEepromRuntimeDataLocation = EEPROM_RUNTIME_DATA_AREA_1;
    }

    u64EepromRuntimeDataSequence = LastSequence;
}

static void app_storage_eeprom_runtime_data_save(void)
{
    u64EepromRuntimeDataSequence++;
    Eeprom_Runtime_Data_Area_t eLocation = eNextEepromRuntimeDataLocation;

    uint32_t u32RuntimeDataLen = sizeof(Eeprom_Sequence_t) + sizeof(Eeprom_Runtime_Data_t);
    memcpy(&au8StorageBuf[0], &u64EepromRuntimeDataSequence, sizeof(Eeprom_Sequence_t));
    memcpy(&au8StorageBuf[sizeof(Eeprom_Sequence_t)], &sEepromRuntimeData, sizeof(Eeprom_Runtime_Data_t));
    sv_eeprom_write(eLocation, au8StorageBuf, u32RuntimeDataLen);

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
    }
}

static void app_storage_module_serial_load(void)
{
    sv_eeprom_read(EEPROM_DEVICE_SERIAL_ADDRESS, (uint8_t *)&u64ModuleSerial, sizeof(u64ModuleSerial));
}

static void app_storage_config_parameter_init(void)
{
    memset(&sIpEndpoint, 0, sizeof(sIpEndpoint));
    memcpy(sIpEndpoint.au8Ipv4, IPV4_ADDRESS, strlen(IPV4_ADDRESS));
    memcpy(sIpEndpoint.au8Port, IPV4_PORT, strlen(IPV4_PORT));
    sv_eeprom_write(EEPROM_IP_ENDPOINT_ADDRESS, (const uint8_t *)&sIpEndpoint, sizeof(sIpEndpoint));

    sModuleConfig.u16LatchPeriod = LATCH_PERIOD;
    sModuleConfig.u16PushPeriod = PUSH_PERIOD;
    sModuleConfig.fTimezone = TIMEZONE;
    sv_eeprom_write(EEPROM_MODULE_CONFIG_ADDRESS, (const uint8_t *)&sModuleConfig, sizeof(sModuleConfig));

    for (uint8_t i = 0; i < MAX_PULSE_METER_COUNT; i++)
    {
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
        memset(sPressureSensorConfig[i].u8Serial, 0, sizeof(sPressureSensorConfig[i].u8Serial));

        sPressureSensorConfig[i].sConfig.fMinCurrent = SENSOR_MIN_CURRENT;
        sPressureSensorConfig[i].sConfig.fMaxCurrent = SENSOR_MAX_CURRENT;
        sPressureSensorConfig[i].sConfig.fMinPressure = SENSOR_MIN_PRESSURE;
        sPressureSensorConfig[i].sConfig.fMaxPressure = SENSOR_MAX_PRESSURE;
    }
    sv_eeprom_write(EEPROM_PRESSURE_SENSOR_CONFIG_ADDRESS, (const uint8_t *)sPressureSensorConfig, sizeof(sPressureSensorConfig));
}

static void app_storage_config_parameter_load(void)
{
    sv_eeprom_read(EEPROM_IP_ENDPOINT_ADDRESS, (uint8_t *)&sIpEndpoint, sizeof(sIpEndpoint));
    sv_eeprom_read(EEPROM_MODULE_CONFIG_ADDRESS, (uint8_t *)&sModuleConfig, sizeof(sModuleConfig));
    sv_eeprom_read(EEPROM_PULSE_METER_CONFIG_ADDRESS, (uint8_t *)sPulseMeterConfig, sizeof(sPulseMeterConfig));
    sv_eeprom_read(EEPROM_MODBUS_METER_CONFIG_ADDRESS, (uint8_t *)sModbusMeterConfig, sizeof(sModbusMeterConfig));
    sv_eeprom_read(EEPROM_PRESSURE_SENSOR_CONFIG_ADDRESS, (uint8_t *)sPressureSensorConfig, sizeof(sPressureSensorConfig));
}