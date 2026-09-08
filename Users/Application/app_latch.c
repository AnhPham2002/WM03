#include "app_latch.h"
#include "sv_modbus_meter.h"
#include "sv_pressure_sensor.h"

static Latch_Data_t sLatchData;
static volatile bool bLatchPeriodFlag = false;

void app_latch_execute(void)
{
    static Latch_Step_t eStep = LATCH_STEP_IDLE;
    static uint8_t u8Index = 0;

    switch (eStep)
    {
    case LATCH_STEP_IDLE:
    {
        if (bLatchPeriodFlag)
        {
            Date_Time_t sDateTime;

            bLatchPeriodFlag = false;
            u8Index = 0;

            sv_time_get_date_time(&sDateTime);
            memcpy(&sLatchData.sDateTime, &sDateTime, sizeof(Date_Time_t));

            eStep = LATCH_STEP_READ_PULSE_METER;
            sys_log((const uint8_t *)"TIME\r\n", 6);
        }
        break;
    }

    case LATCH_STEP_READ_PULSE_METER:
    {
        Pulse_Meter_Data_t sPulseMeterData;

        if (!sv_pulse_meter_get_data(u8Index, &sPulseMeterData))
        {
            sPulseMeterData.dTotalForward = -1;
            sPulseMeterData.dTotalReverse = -1;
            sPulseMeterData.dFlowRate = -1;
        }
        memcpy(&sLatchData.sPulseMeter[u8Index], &sPulseMeterData, sizeof(Pulse_Meter_Data_t));

        u8Index++;
        if (u8Index >= MAX_PULSE_METER_COUNT)
        {
            u8Index = 0;
            eStep = LATCH_STEP_READ_MODBUS_METER;
        }
        sys_log((const uint8_t *)"PULSE\r\n", 7);

        break;
    }

    case LATCH_STEP_READ_MODBUS_METER:
    {
        Modbus_Meter_Data_t sModbusMeterData;

        if (!sv_modbus_meter_get_data(u8Index, &sModbusMeterData))
        {
            sModbusMeterData.dTotalForward = -1;
            sModbusMeterData.dTotalReverse = -1;
            sModbusMeterData.dFlowRate = -1;
        }
        memcpy(&sLatchData.sModbusMeter[u8Index], &sModbusMeterData, sizeof(Modbus_Meter_Data_t));

        u8Index++;
        if (u8Index >= MAX_MODBUS_METER_COUNT)
        {
            u8Index = 0;
            eStep = LATCH_STEP_READ_PRESSURE_SENSOR;
        }
        sys_log((const uint8_t *)"MODBUS\r\n", 8);

        break;
    }

    case LATCH_STEP_READ_PRESSURE_SENSOR:
    {
        Pressure_Sensor_Data_t sPressureSensorData;

        if (!sv_pressure_sensor_get_data(u8Index, &sPressureSensorData))
        {
            sPressureSensorData.fPressure = -1;
        }
        memcpy(&sLatchData.sPressureSensor[u8Index], &sPressureSensorData, sizeof(Pressure_Sensor_Data_t));

        u8Index++;
        if (u8Index >= MAX_PRESSURE_SENSOR_COUNT)
        {
            u8Index = 0;
            eStep = LATCH_STEP_SAVE_EEPROM;
        }
        sys_log((const uint8_t *)"PRESSURE\r\n", 10);

        break;
    }

    case LATCH_STEP_SAVE_EEPROM:
        app_storage_latch_save(&sLatchData);
        eStep = LATCH_STEP_IDLE;
        sys_log((const uint8_t *)"OK\r\n", 4);
        break;

    default:
        bLatchPeriodFlag = false;
        u8Index = 0;
        eStep = LATCH_STEP_IDLE;
        break;
    }
}

void app_latch_activate(void)
{
    bLatchPeriodFlag = true;
}