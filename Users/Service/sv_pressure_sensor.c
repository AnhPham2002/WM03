#include "sv_pressure_sensor.h"

static Pressure_Sensor_Config_t sPressureSensorConfig[MAX_PRESSURE_SENSOR_COUNT];

/*==================================================================================================
*                                PRIVATE FUNCTIONS DECLARATIONS
==================================================================================================*/

/**
 * @brief Select pressure sensor channel.
 *
 * @param[in] eSensor Pressure sensor selection.
 *
 * @return true if the sensor channel is selected successfully, otherwise false.
 */
bool sv_pressure_sensor_select_channel(Pressure_Sensor_Select_t eSensor);

/*==================================================================================================
*                                   PUBLIC FUNCTIONS DEFINITIONS
==================================================================================================*/

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

    memcpy(&sPressureSensorConfig[u8MeterIndex], pConfig, sizeof(Pressure_Sensor_Config_t));
    return true;
}

bool sv_pressure_sensor_get_data(uint8_t u8MeterIndex, Pressure_Sensor_Data_t *pData)
{
    float fVoltage;
    float fCurrent;

    if ((u8MeterIndex >= MAX_PRESSURE_SENSOR_COUNT) || (sPressureSensorConfig[u8MeterIndex].fMinCurrent >= sPressureSensorConfig[u8MeterIndex].fMaxCurrent) ||
        (sPressureSensorConfig[u8MeterIndex].fMinPressure >= sPressureSensorConfig[u8MeterIndex].fMaxPressure) || (pData == NULL))
    {
        return false;
    }

    if (!sv_pressure_sensor_select_channel(u8MeterIndex))
    {
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

    pData->fPressure = sPressureSensorConfig[u8MeterIndex].fMinPressure +
                       ((fCurrent - sPressureSensorConfig[u8MeterIndex].fMinCurrent) * (sPressureSensorConfig[u8MeterIndex].fMaxPressure - sPressureSensorConfig[u8MeterIndex].fMinPressure) /
                        (sPressureSensorConfig[u8MeterIndex].fMaxCurrent - sPressureSensorConfig[u8MeterIndex].fMinCurrent)); // bar

    drv_ext_adc_disable_all_sensors();
    drv_ext_adc_pwr_off();

    return true;
}

/*==================================================================================================
*                                   PRIVATE FUNCTIONS DEFINITIONS
==================================================================================================*/

bool sv_pressure_sensor_select_channel(Pressure_Sensor_Select_t eSensor)
{
    switch (eSensor)
    {
    case PRESSURE_SENSOR_1:
        drv_ext_adc_enable_sensor1();
        return true;

    case PRESSURE_SENSOR_2:
        drv_ext_adc_enable_sensor2();
        return true;

    default:
        return false;
    }
}