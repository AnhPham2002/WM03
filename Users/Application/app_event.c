#include "app_event.h"

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
 * @brief Monitor power status and record power events.
 *
 * Confirms a power status change after the configured confirmation time
 * before recording the corresponding power detection or loss event.
 */
static void app_event_power(void);

/**
 * @brief Monitor battery voltage and record battery events.
 *
 * Confirms low battery and recovery conditions for the configured confirmation
 * time before recording the corresponding battery event.
 */
static void app_event_battery(void);

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
    app_event_battery();
    app_event_config_changed();
    app_event_password_changed();
    app_event_firmware_update();
}

/*==================================================================================================
*                                   PRIVATE FUNCTIONS DEFINITIONS
==================================================================================================*/

static void app_event_magnetic(void)
{
    static bool bMagneticActive = false;
    static uint32_t u32MagneticDetectTime = 0;
    static uint32_t u32MagneticEndTime = 0;

    bool bMagneticStatus = sv_485_communication_magnetic_detect();

    if (!bMagneticActive)
    {
        if (bMagneticStatus)
        {
            if (u32MagneticDetectTime == 0)
            {
                u32MagneticDetectTime = sys_time_ms();
            }
            else if (sys_time_ms() - u32MagneticDetectTime >= MAGNETIC_DETECT_MIN_TIME_MS)
            {
                bMagneticActive = true;
                u32MagneticDetectTime = 0;

                sv_time_get_date_time(&sEventData.sDateTime);
                sEventData.u8MeterType = MODULE_TYPE;
                sEventData.u8MeterIndex = 0;
                sEventData.u8EventCode = EVENT_MAGNETIC_DETECTED;
                app_storage_event_save(&sEventData);
            }
        }
        else
        {
            u32MagneticDetectTime = 0;
        }
    }
    else
    {
        if (bMagneticStatus)
        {
            u32MagneticEndTime = 0;
        }
        else
        {
            if (u32MagneticEndTime == 0)
            {
                u32MagneticEndTime = sys_time_ms();
            }
            else if (sys_time_ms() - u32MagneticEndTime >= MAGNETIC_END_DELAY_MS)
            {
                bMagneticActive = false;
                u32MagneticEndTime = 0;

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
    // static bool bPowerDetected = false;
    // static uint32_t u32PowerTime = 0;

    // bool bPowerStatus = sv_power_is_detected();

    // if (bPowerStatus != bPowerDetected)
    // {
    //     if (u32PowerTime == 0)
    //     {
    //         u32PowerTime = sys_time_ms();
    //     }
    //     else if (sys_time_ms() - u32PowerTime >= POWER_STATUS_CONFIRM_TIME_MS)
    //     {
    //         bPowerDetected = bPowerStatus;
    //         u32PowerTime = 0;

    //         sv_time_get_date_time(&sEventData.sDateTime);
    //         sEventData.u8MeterType = MODULE_TYPE;
    //         sEventData.u8MeterIndex = 0;
    //         sEventData.u8EventCode = bPowerDetected ? EVENT_POWER_DETECTED : EVENT_POWER_LOST;
    //         app_storage_event_save(&sEventData);
    //     }
    // }
    // else
    // {
    //     u32PowerTime = 0;
    // }
}

static void app_event_battery(void)
{
    //     static bool bLowBattery = false;
    //     static uint32_t u32BatteryTime = 0;

    //     uint32_t u32BatteryVoltage = sv_battery_get_voltage_mv();

    //     if (!bLowBattery)
    //     {
    //         if (u32BatteryVoltage <= BATTERY_LOW_THRESHOLD_MV)
    //         {
    //             if (u32BatteryTime == 0)
    //             {
    //                 u32BatteryTime = sys_time_ms();
    //             }
    //             else if (sys_time_ms() - u32BatteryTime >= BATTERY_STATUS_CONFIRM_TIME_MS)
    //             {
    //                 bLowBattery = true;
    //                 u32BatteryTime = 0;

    //                 sv_time_get_date_time(&sEventData.sDateTime);
    //                 sEventData.u8MeterType = MODULE_TYPE;
    //                 sEventData.u8MeterIndex = 0;
    //                 sEventData.u8EventCode = EVENT_LOW_BATTERY;
    //                 app_storage_event_save(&sEventData);
    //             }
    //         }
    //         else
    //         {
    //             u32BatteryTime = 0;
    //         }
    //     }
    //     else
    //     {
    //         if (u32BatteryVoltage >= BATTERY_LOW_RECOVERY_MV)
    //         {
    //             if (u32BatteryTime == 0)
    //             {
    //                 u32BatteryTime = sys_time_ms();
    //             }
    //             else if (sys_time_ms() - u32BatteryTime >= BATTERY_STATUS_CONFIRM_TIME_MS)
    //             {
    //                 bLowBattery = false;
    //                 u32BatteryTime = 0;

    //                 sv_time_get_date_time(&sEventData.sDateTime);
    //                 sEventData.u8MeterType = MODULE_TYPE;
    //                 sEventData.u8MeterIndex = 0;
    //                 sEventData.u8EventCode = EVENT_LOW_BATTERY_RECOVERED;
    //                 app_storage_event_save(&sEventData);
    //             }
    //         }
    //         else
    //         {
    //             u32BatteryTime = 0;
    //         }
    //     }
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