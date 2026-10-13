#include "app_main.h"

#define UNACTIVATED_SHUTDOWN_TIMEOUT_SECOND 10
uint32_t u32TimestampUnactiveStart;

bool bInitialized = false;

static uint8_t au8RxTest[1000];
static uint16_t u16RxTestLen;

void app_main(void)
{
    drv_wdt_init();

    drv_rs485_init();

    sv_485_communication_init();

    sys_debug_init();

    sv_time_init();

    app_storage_init();

    app_storage_increase_reset_count();

    if (sys_get_vector_table_address() == SLOT_A_START_ADDR)
    {
        sys_log((const uint8_t *)"System Init Firmware A\n", 23);
    }
    else
    {
        sys_log((const uint8_t *)"System Init Firmware B\n", 23);
    }

    u32TimestampUnactiveStart = sv_time_get_unix_timestamp();

    while (1)
    {
        if (sys_console(au8RxTest, &u16RxTestLen))
        {
        }

        memset(au8RxTest, 0, sizeof(au8RxTest));

        app_protocol_update();
        app_485_communication_execute();

        drv_wdt_clear_refresh_count();
        sv_charge_wd_rst();

        if ((!app_485_communication_in_progress()) && (!app_cellular_communication_in_progress()))
        {
            sys_sleep();
        }

        if (app_485_communication_in_progress())
        {
            u32TimestampUnactiveStart = sv_time_get_unix_timestamp();
        }

        if (app_storage_get_activation_status() == DEVICE_ACTIVATION_STATUS_DEACTIVED)
        {
            if (sv_time_get_unix_timestamp() - u32TimestampUnactiveStart > UNACTIVATED_SHUTDOWN_TIMEOUT_SECOND)
            {
                sv_charge_disconnect_battery();
            }
        }
        else if (app_storage_get_activation_status() == DEVICE_ACTIVATION_STATUS_ACTIVED)
        {
            if (!bInitialized)
            {
                bInitialized = true;

                sv_pulse_meter_init();
                sv_pressure_sensor_init();
                app_cellular_init();
            }

            app_event_execute();
            app_latch_execute();
            app_push_execute();

            u32TimestampUnactiveStart = sv_time_get_unix_timestamp();
        }
    }
}