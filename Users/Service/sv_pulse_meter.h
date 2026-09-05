#pragma once

#include "drv_pulse.h"

#define MAX_PULSE_METER_COUNT 4

typedef struct __attribute__((packed))
{
	uint16_t u16PulseFactor; // pulse / m3
	Pulse_Config_t sPulseConfig;
} Pulse_Meter_Config_t;

typedef struct
{
    double dTotalForward;
    double dTotalReverse;
    double dFlowRate;
} Pulse_Meter_Data_t;

/**
 * @brief Initialize pulse meter service.
 *
 * @param[in] pCount Default pulse count.
 */
void sv_pulse_meter_init(const Pulse_Count_t *pCount);

/**
 * @brief Set pulse meter count.
 *
 * @param[in] u8MeterIndex Meter index.
 * @param[in] pData        Pulse meter data.
 *
 * @return true if the count is set successfully, otherwise false.
 */
bool sv_pulse_meter_set_count(uint8_t u8MeterIndex, const Pulse_Meter_Data_t *pData);

/**
 * @brief Set pulse meter configuration.
 *
 * @param[in] u8MeterIndex Meter index.
 * @param[in] pConfig      Pulse meter configuration.
 *
 * @return true if the configuration is set successfully, otherwise false.
 */
bool sv_pulse_meter_set_config(uint8_t u8MeterIndex, const Pulse_Meter_Config_t *pConfig);


/**
 * @brief Get pulse meter data.
 *
 * @param[in]  u8MeterIndex Meter index.
 * @param[out] pData        Pulse meter data.
 *
 * @return true if the data is retrieved successfully, otherwise false.
 */
bool sv_pulse_meter_get_data(uint8_t u8MeterIndex, Pulse_Meter_Data_t *pData);