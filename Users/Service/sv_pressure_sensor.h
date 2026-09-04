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
 * @brief Get pressure sensor value.
 *
 * @param[in]  eSensor   Pressure sensor selection.
 * @param[out] pPressure Pressure value in bar.
 *
 * @return true if pressure is read successfully, otherwise false.
 */
bool sv_pressure_sensor_get_data(Pressure_Sensor_Select_t eSensor, float *pPressure);