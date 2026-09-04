#include "sv_pressure_sensor.h"

static Pressure_Sensor_Config_t sPressureSensorConfig[MAX_PRESSURE_SENSOR_COUNT];

void sv_pressure_sensor_init(void)
{
    drv_ext_adc_init();
}

bool sv_pressure_sensor_set_config(uint8_t u8MeterIndex, const Pressure_Sensor_Config_t *pConfig)
{
    if ((u8MeterIndex > MAX_PRESSURE_SENSOR_COUNT) || (pConfig == NULL))
    {
        return false;
    }

    sPressureSensorConfig[u8MeterIndex] = *pConfig;
    return true;
}

bool sv_pressure_sensor_get_data(Pressure_Sensor_Select_t eSensor, float *pPressure)
{
    float fVoltage;
    float fCurrent;

    if ((eSensor >= MAX_PRESSURE_SENSOR_COUNT) || (sPressureSensorConfig[eSensor].fMinCurrent >= sPressureSensorConfig[eSensor].fMaxCurrent) ||
        (sPressureSensorConfig[eSensor].fMinPressure >= sPressureSensorConfig[eSensor].fMaxPressure) || (pPressure == NULL))
    {
        return false;
    }

    switch (eSensor)
    {
    case PRESSURE_SENSOR_1:
        drv_ext_adc_enable_sensor1();
        break;

    case PRESSURE_SENSOR_2:
        drv_ext_adc_enable_sensor2();
        break;

    default:
        return false;
    }

    drv_ext_adc_pwr_on();

    if (!drv_ext_adc_read(&fVoltage))
    {
        drv_ext_adc_disable_all_sensors();
        drv_ext_adc_pwr_off();
        return false;
    }

    fCurrent = fVoltage * 1000.0f / SHUNT_RESISTOR_ADC; // mA

    *pPressure = sPressureSensorConfig[eSensor].fMinPressure +
                 ((fCurrent - sPressureSensorConfig[eSensor].fMinCurrent) * (sPressureSensorConfig[eSensor].fMaxPressure - sPressureSensorConfig[eSensor].fMinPressure) /
                  (sPressureSensorConfig[eSensor].fMaxCurrent - sPressureSensorConfig[eSensor].fMinCurrent)); // bar

    drv_ext_adc_disable_all_sensors();
    drv_ext_adc_pwr_off();

    return true;
}