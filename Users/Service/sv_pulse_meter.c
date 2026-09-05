#include "sv_pulse_meter.h"

static uint16_t u16PulseFactor[MAX_PULSE_METER_COUNT];

void sv_pulse_meter_init(const Pulse_Count_t *pCount)
{
    drv_pulse_init(pCount);
}

bool sv_pulse_meter_set_count(uint8_t u8MeterIndex, const Pulse_Meter_Data_t *pData)
{
    if ((u8MeterIndex >= MAX_PULSE_METER_COUNT) || (pData == NULL))
    {
        return false;
    }

    Pulse_Count_t sPulseCount;
    sPulseCount.u64ForwardPulseCount = (uint64_t)(pData->dTotalForward * u16PulseFactor[u8MeterIndex] + 0.5);
    sPulseCount.u64ReversePulseCount = (uint64_t)(pData->dTotalReverse * u16PulseFactor[u8MeterIndex] + 0.5);
    return drv_pulse_set_count(u8MeterIndex, &sPulseCount);
}

bool sv_pulse_meter_set_config(uint8_t u8MeterIndex, const Pulse_Meter_Config_t *pConfig)
{
    if ((u8MeterIndex >= MAX_PULSE_METER_COUNT) || (pConfig == NULL) || (pConfig->u16PulseFactor == 0))
    {
        return false;
    }

    if (!drv_pulse_set_config(u8MeterIndex, &pConfig->sPulseConfig))
    {
        return false;
    }

    u16PulseFactor[u8MeterIndex] = pConfig->u16PulseFactor;
    return true;
}

bool sv_pulse_meter_get_data(uint8_t u8MeterIndex, Pulse_Meter_Data_t *pData)
{
    if ((u8MeterIndex >= MAX_PULSE_METER_COUNT) || (pData == NULL) || (u16PulseFactor[u8MeterIndex] == 0))
    {
        return false;
    }

    Pulse_Data_t sPulseData;

    if (!drv_pulse_get_data(u8MeterIndex, &sPulseData))
    {
        return false;
    }

    pData->dTotalForward = (double)sPulseData.u64ForwardPulseCount / (double)u16PulseFactor[u8MeterIndex];
    pData->dTotalReverse = (double)sPulseData.u64ReversePulseCount / (double)u16PulseFactor[u8MeterIndex];
    pData->dFlowRate = sPulseData.dPulseFrequency / (double)u16PulseFactor[u8MeterIndex];

    return true;
}