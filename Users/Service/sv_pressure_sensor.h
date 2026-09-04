#pragma once

#include "drv_ext_adc.h"

#define MAX_PRESSURE_SENSOR_COUNT 2

#define SHUNT_RESISTOR_ADC 25 // Ohm

typedef enum
{
    PRESSURE_SENSOR_1 = 0,
    PRESSURE_SENSOR_2
} Pressure_Sensor_Select_t;

typedef struct
{
    float fMinCurrent; // mA
    float fMaxCurrent; // mA
    float fMinPressure; // bar
    float fMaxPressure; // bar
} Pressure_Sensor_Config_t;

typedef struct
{
    float fPressure;
} Pressure_Sensor_Data_t;

/**
 * @brief Initialize pressure sensor service.
 */
void sv_pressure_sensor_init(void);

/**
 * @brief Set pressure sensor configuration.
 *
 * @param[in] u8MeterIndex Meter index.
 * @param[in] pConfig      Pressure sensor configuration.
 *
 * @return true if the configuration is set successfully, otherwise false.
 */
bool sv_pressure_sensor_set_config(uint8_t u8MeterIndex, const Pressure_Sensor_Config_t *pConfig);

/**
 * @brief Read pressure sensor data.
 *
 * Reads the selected pressure sensor voltage and converts it to pressure
 * based on the configured current and pressure ranges.
 *
 * @param[in]  u8MeterIndex Pressure sensor index.
 * @param[out] pData        Pressure sensor data.
 *
 * @return true if the pressure data is read successfully, otherwise false.
 */
bool sv_pressure_sensor_get_data(uint8_t u8MeterIndex, Pressure_Sensor_Data_t *pData);