#include "app_event.h"

#define RAM_NOINIT_MAGNETIC_MAGIC_NUMBER 0x11223344
#define RAM_NOINIT_POWER_MAGIC_NUMBER 0x5A5AA5A5

typedef struct
{
    uint32_t u32Magic;
    bool bMagneticActive;
    uint32_t u32MagneticDetectTimestamp;
    uint32_t u32MagneticEndTimestamp;
} Magnetic_Status_t;

typedef struct
{
    uint32_t u32Magic;
    bool bVbusPresent;
    bool bVbusChecking;
    uint32_t u32VbusStartCheckTimestamp;
    uint32_t u32BatteryCheckTimestamp;
    bool bBatteryLow;
} Power_Status_t;

static Magnetic_Status_t sMagneticStatus RAM_NOINIT;
static Power_Status_t sPowerStatus RAM_NOINIT;

static Event_Data_t sEventData;

/*==================================================================================================
*                                PRIVATE FUNCTIONS DECLARATIONS
==================================================================================================*/

/**
 * @brief Monitor magnetic field status and record magnetic events.
 *
 * Confirms magnetic field detection and release using the configured
 * detection and release delays before recording the corresponding events.
 */
static void app_event_magnetic(void);

/**
 * @brief Monitor power and battery status and record events.
 *
 * Monitors VBUS connection and disconnection using the charger interrupt
 * and confirms the status change after the configured confirmation time.
 * Monitors battery voltage periodically and records low battery and
 * recovery events after the configured confirmation time.
 */
static void app_event_power(void);

/**
 * @brief Record a configuration change event.
 *
 * Records a configuration change event only once and updates the
 * configuration change timestamp.
 */
static void app_event_config_changed(void);

/**
 * @brief Record a password change event.
 */
static void app_event_password_changed(void);

/**
 * @brief Record a firmware update detection event.
 */
static void app_event_firmware_update(void);

/*==================================================================================================
*                                   PUBLIC FUNCTIONS DEFINITIONS
==================================================================================================*/

void app_event_execute(void)
{
    app_event_magnetic();
    app_event_power();
    app_event_config_changed();
    app_event_password_changed();
    app_event_firmware_update();
}

/*==================================================================================================
*                                   PRIVATE FUNCTIONS DEFINITIONS
==================================================================================================*/

static void app_event_magnetic(void)
{
    bool bMagneticStatus = sv_485_communication_magnetic_detect();
    uint32_t u32TimeNow = sv_time_get_unix_timestamp();

    if (sMagneticStatus.u32Magic != RAM_NOINIT_MAGNETIC_MAGIC_NUMBER)
    {
        sMagneticStatus.bMagneticActive = false;
        sMagneticStatus.u32MagneticDetectTimestamp = 0;
        sMagneticStatus.u32MagneticEndTimestamp = 0;
        sMagneticStatus.u32Magic = RAM_NOINIT_MAGNETIC_MAGIC_NUMBER;
    }

    if (!sMagneticStatus.bMagneticActive)
    {
        sMagneticStatus.u32MagneticEndTimestamp = 0;

        if (bMagneticStatus)
        {
            if (sMagneticStatus.u32MagneticDetectTimestamp == 0)
            {
                sMagneticStatus.u32MagneticDetectTimestamp = u32TimeNow;
            }
            else if (u32TimeNow - sMagneticStatus.u32MagneticDetectTimestamp >= MAGNETIC_DETECT_MIN_TIME_SECOND)
            {
                sMagneticStatus.bMagneticActive = true;
                sMagneticStatus.u32MagneticDetectTimestamp = 0;

                sv_time_get_date_time(&sEventData.sDateTime);
                sEventData.u8MeterType = MODULE_TYPE;
                sEventData.u8MeterIndex = 0;
                sEventData.u8EventCode = EVENT_MAGNETIC_DETECTED;
                app_storage_event_save(&sEventData);
            }
        }
        else
        {
            sMagneticStatus.u32MagneticDetectTimestamp = 0;
        }
    }
    else
    {
        sMagneticStatus.u32MagneticDetectTimestamp = 0;

        if (bMagneticStatus)
        {
            sMagneticStatus.u32MagneticEndTimestamp = 0;
        }
        else
        {
            if (sMagneticStatus.u32MagneticEndTimestamp == 0)
            {
                sMagneticStatus.u32MagneticEndTimestamp = u32TimeNow;
            }
            else if (u32TimeNow - sMagneticStatus.u32MagneticEndTimestamp >= MAGNETIC_END_DELAY_SECOND)
            {
                sMagneticStatus.bMagneticActive = false;
                sMagneticStatus.u32MagneticEndTimestamp = 0;

                sv_time_get_date_time(&sEventData.sDateTime);
                sEventData.u8MeterType = MODULE_TYPE;
                sEventData.u8MeterIndex = 0;
                sEventData.u8EventCode = EVENT_MAGNETIC_ENDED;
                app_storage_event_save(&sEventData);
            }
        }
    }
}

static void app_event_power(void)
{
    bool bVbusStatus;
    uint8_t u8ChargingStatus;
    uint16_t u16BatteryVoltageMv;
    uint32_t u32TimestampNow = sv_time_get_unix_timestamp();

    if (sPowerStatus.u32Magic != RAM_NOINIT_POWER_MAGIC_NUMBER)
    {
        sv_charge_get_power_info(&bVbusStatus, &u8ChargingStatus, &u16BatteryVoltageMv);

        sPowerStatus.bVbusPresent = bVbusStatus;
        sPowerStatus.bVbusChecking = false;
        sPowerStatus.u32VbusStartCheckTimestamp = 0;
        sPowerStatus.bBatteryLow = (u16BatteryVoltageMv <= BATTERY_LOW_THRESHOLD_MV);
        sPowerStatus.u32BatteryCheckTimestamp = u32TimestampNow;
        sPowerStatus.u32Magic = RAM_NOINIT_POWER_MAGIC_NUMBER;
    }

    /* VBUS event */
    if (sv_charge_interrupt_detected() && !sPowerStatus.bVbusChecking)
    {
        sPowerStatus.bVbusChecking = true;
        sPowerStatus.u32VbusStartCheckTimestamp = u32TimestampNow;
    }

    if (sPowerStatus.bVbusChecking && (u32TimestampNow - sPowerStatus.u32VbusStartCheckTimestamp >= POWER_STATUS_CONFIRM_SECOND))
    {
        sv_charge_get_power_info(&bVbusStatus, &u8ChargingStatus, &u16BatteryVoltageMv);

        sPowerStatus.bVbusChecking = false;
        sPowerStatus.u32VbusStartCheckTimestamp = 0;

        if (bVbusStatus != sPowerStatus.bVbusPresent)
        {
            sPowerStatus.bVbusPresent = bVbusStatus;

            sv_time_get_date_time(&sEventData.sDateTime);
            sEventData.u8MeterType = MODULE_TYPE;
            sEventData.u8MeterIndex = 0;
            sEventData.u8EventCode = bVbusStatus ? EVENT_POWER_CONNECTED : EVENT_POWER_LOST;
            app_storage_event_save(&sEventData);
        }
    }

    /* Battery event */
    if (u32TimestampNow - sPowerStatus.u32BatteryCheckTimestamp >= BATTERY_STATUS_CHECK_SECOND)
    {
        sPowerStatus.u32BatteryCheckTimestamp = u32TimestampNow;

        sv_charge_get_power_info(&bVbusStatus, &u8ChargingStatus, &u16BatteryVoltageMv);

        if (!sPowerStatus.bBatteryLow && (u16BatteryVoltageMv <= BATTERY_LOW_THRESHOLD_MV))
        {
            sPowerStatus.bBatteryLow = true;

            sv_time_get_date_time(&sEventData.sDateTime);
            sEventData.u8MeterType = MODULE_TYPE;
            sEventData.u8MeterIndex = 0;
            sEventData.u8EventCode = EVENT_LOW_BATTERY;
            app_storage_event_save(&sEventData);
        }
        else if (sPowerStatus.bBatteryLow && (u16BatteryVoltageMv >= BATTERY_LOW_RECOVERY_MV))
        {
            sPowerStatus.bBatteryLow = false;

            sv_time_get_date_time(&sEventData.sDateTime);
            sEventData.u8MeterType = MODULE_TYPE;
            sEventData.u8MeterIndex = 0;
            sEventData.u8EventCode = EVENT_LOW_BATTERY_ENDED;
            app_storage_event_save(&sEventData);
        }
    }
}

static void app_event_config_changed(void)
{
    // static bool bConfigChanged = false;
    // static uint32_t u32ConfigChangeTime = 0;

    // if (!bConfigChanged)
    // {
    //     bConfigChanged = true;

    //     sv_time_get_date_time(&sEventData.sDateTime);
    //     sEventData.u8MeterType = MODULE_TYPE;
    //     sEventData.u8MeterIndex = 0;
    //     sEventData.u8EventCode = EVENT_CONFIG_CHANGED;
    //     app_storage_event_save(&sEventData);
    // }

    // u32ConfigChangeTime = sys_time_ms();
}

static void app_event_password_changed(void)
{
    // sv_time_get_date_time(&sEventData.sDateTime);
    // sEventData.u8MeterType = MODULE_TYPE;
    // sEventData.u8MeterIndex = 0;
    // sEventData.u8EventCode = EVENT_PASSWORD_CHANGED;
    // app_storage_event_save(&sEventData);
}

static void app_event_firmware_update(void) {}