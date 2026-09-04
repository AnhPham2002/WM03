#include "sv_modbus_meter.h"

static bool bModbusLatchPeriod = false;
static Modbus_Meter_Config_t sModbusMeterConfig[MAX_MODBUS_METER_COUNT];

/*==================================================================================================
*                                PRIVATE FUNCTIONS DECLARATIONS
==================================================================================================*/

/**
 * @brief Initialize Modbus meter.
 *
 * @param[in] u8MeterIndex Meter index.
 *
 * @return true if initialization is successful, otherwise false.
 */
static bool sv_modbus_meter_init(uint8_t u8MeterIndex);

/**
 * @brief Get Modbus meter data size.
 *
 * @param[in] u8DataType Data type.
 *
 * @return Data size in bytes, or 0 if the data type is invalid.
 */
static uint8_t sv_modbus_meter_data_size(uint8_t u8DataType);

/**
 * @brief Decode Modbus meter data.
 *
 * @param[in]  pDataIn      Input data buffer.
 * @param[in]  u8DataType   Data type.
 * @param[in]  u8WordSwap   Word swap enable.
 * @param[in]  s8Multiplier Data multiplier exponent.
 * @param[out] pDataOut     Decoded data value.
 *
 * @return true if decoding is successful, otherwise false.
 */
static bool sv_modbus_meter_decode(const uint8_t *pDataIn, uint8_t u8DataType, uint8_t u8WordSwap, int8_t s8Multiplier, double *pDataOut);

/*==================================================================================================
*                                   PUBLIC FUNCTIONS DEFINITIONS
==================================================================================================*/

void sv_modbus_meter_set_latch_period(void)
{
    bModbusLatchPeriod = true;
}

bool sv_modbus_meter_set_config(uint8_t u8MeterIndex, const Modbus_Meter_Config_t *pConfig)
{
    if ((u8MeterIndex > MAX_MODBUS_METER_COUNT) || (pConfig == NULL))
    {
        return false;
    }

    memcpy(&sModbusMeterConfig[u8MeterIndex], pConfig, sizeof(Modbus_Meter_Config_t));
    return true;
}

bool sv_modbus_meter_get_data(uint8_t u8MeterIndex, Modbus_Meter_Data_t *pData)
{
    uint8_t au8RawData[8];
    uint16_t u16RawDataSize;
    Modbus_Status_t eModbusStatus;

    if ((u8MeterIndex >= MAX_MODBUS_METER_COUNT) || (pData == NULL))
    {
        return false;
    }

    if (!sv_modbus_meter_init(u8MeterIndex))
    {
        return false;
    }

    if (sModbusMeterConfig[u8MeterIndex].sForwardTotalizer.u8ParameterEnable)
    {
        eModbusStatus = drv_modbus_master_read_register(sModbusMeterConfig[u8MeterIndex].sModbusConfig.u8SlaveAddress, sModbusMeterConfig[u8MeterIndex].sModbusConfig.u8ReadFuncCode,
                                                        sModbusMeterConfig[u8MeterIndex].sForwardTotalizer.u16RegisterAddress,
                                                        sv_modbus_meter_data_size(sModbusMeterConfig[u8MeterIndex].sForwardTotalizer.u8DataType) / 2, au8RawData, &u16RawDataSize);
        if (eModbusStatus == MODBUS_OK)
        {
            sv_modbus_meter_decode(au8RawData, sModbusMeterConfig[u8MeterIndex].sForwardTotalizer.u8DataType, sModbusMeterConfig[u8MeterIndex].sForwardTotalizer.u8WordSwap,
                                   sModbusMeterConfig[u8MeterIndex].sForwardTotalizer.s8Multiplier, &pData->dTotalForward);
        }
        else
        {
            pData->dTotalForward = -1; // Invalid value
        }
    }
    else
    {
        pData->dTotalForward = -1; // Don't use
    }

    if (sModbusMeterConfig[u8MeterIndex].sReverseTotalizer.u8ParameterEnable)
    {
        eModbusStatus = drv_modbus_master_read_register(sModbusMeterConfig[u8MeterIndex].sModbusConfig.u8SlaveAddress, sModbusMeterConfig[u8MeterIndex].sModbusConfig.u8ReadFuncCode,
                                                        sModbusMeterConfig[u8MeterIndex].sReverseTotalizer.u16RegisterAddress,
                                                        sv_modbus_meter_data_size(sModbusMeterConfig[u8MeterIndex].sReverseTotalizer.u8DataType) / 2, au8RawData, &u16RawDataSize);
        if (eModbusStatus == MODBUS_OK)
        {
            sv_modbus_meter_decode(au8RawData, sModbusMeterConfig[u8MeterIndex].sReverseTotalizer.u8DataType, sModbusMeterConfig[u8MeterIndex].sReverseTotalizer.u8WordSwap,
                                   sModbusMeterConfig[u8MeterIndex].sReverseTotalizer.s8Multiplier, &pData->dTotalReverse);
        }
        else
        {
            pData->dTotalReverse = -1; // Invalid value
        }
    }
    else
    {
        pData->dTotalReverse = -1; // Don't use
    }

    if (sModbusMeterConfig[u8MeterIndex].sFlowRate.u8ParameterEnable)
    {
        eModbusStatus = drv_modbus_master_read_register(sModbusMeterConfig[u8MeterIndex].sModbusConfig.u8SlaveAddress, sModbusMeterConfig[u8MeterIndex].sModbusConfig.u8ReadFuncCode,
                                                        sModbusMeterConfig[u8MeterIndex].sFlowRate.u16RegisterAddress,
                                                        sv_modbus_meter_data_size(sModbusMeterConfig[u8MeterIndex].sFlowRate.u8DataType) / 2, au8RawData, &u16RawDataSize);
        if (eModbusStatus == MODBUS_OK)
        {
            sv_modbus_meter_decode(au8RawData, sModbusMeterConfig[u8MeterIndex].sFlowRate.u8DataType, sModbusMeterConfig[u8MeterIndex].sFlowRate.u8WordSwap,
                                   sModbusMeterConfig[u8MeterIndex].sFlowRate.s8Multiplier, &pData->dFlowRate);
        }
        else
        {
            pData->dFlowRate = -1; // Invalid value
        }
    }
    else
    {
        pData->dFlowRate = -1; // Don't use
    }

    return true;
}

/*==================================================================================================
*                                   PRIVATE FUNCTIONS DEFINITIONS
==================================================================================================*/

static bool sv_modbus_meter_init(uint8_t u8MeterIndex)
{
    if (u8MeterIndex >= MAX_MODBUS_METER_COUNT)
    {
        return false;
    }

    uint32_t u32Parity;
    switch (sModbusMeterConfig[u8MeterIndex].sModbusConfig.u8SerialConfig)
    {
    case MODBUS_SERIAL_8N1:
        u32Parity = UART_PARITY_NONE;
        break;

    case MODBUS_SERIAL_8O1:
        u32Parity = UART_PARITY_ODD;
        break;

    case MODBUS_SERIAL_8E1:
        u32Parity = UART_PARITY_EVEN;
        break;

    default:
        return false;
    }

    drv_modbus_init(sModbusMeterConfig[u8MeterIndex].sModbusConfig.u32BaudRate, u32Parity);
    return true;
}

static uint8_t sv_modbus_meter_data_size(uint8_t u8DataType)
{
    switch (u8DataType)
    {
    case MODBUS_INT16:
    case MODBUS_UINT16:
        return 2U;

    case MODBUS_INT32:
    case MODBUS_UINT32:
    case MODBUS_FLOAT:
        return 4U;

    case MODBUS_INT64:
    case MODBUS_UINT64:
    case MODBUS_DOUBLE:
        return 8U;

    default:
        return 0U;
    }
}

static bool sv_modbus_meter_decode(const uint8_t *pDataIn, uint8_t u8DataType, uint8_t u8WordSwap, int8_t s8Multiplier, double *pDataOut)
{
    const uint8_t u8DataSize = sv_modbus_meter_data_size(u8DataType);
    if (u8DataSize == 0)
        return false;

    uint8_t au8Buf[8] = {0};
    memcpy(au8Buf, pDataIn, u8DataSize);

    if (u8WordSwap)
    {
        if (u8DataSize == 4)
        {
            uint8_t u8Temp0 = au8Buf[0];
            uint8_t u8Temp1 = au8Buf[1];
            au8Buf[0] = au8Buf[2];
            au8Buf[1] = au8Buf[3];
            au8Buf[2] = u8Temp0;
            au8Buf[3] = u8Temp1;
        }

        if (u8DataSize == 8)
        {
            uint8_t u8Temp0 = au8Buf[0];
            uint8_t u8Temp1 = au8Buf[1];
            au8Buf[0] = au8Buf[6];
            au8Buf[1] = au8Buf[7];
            au8Buf[6] = u8Temp0;
            au8Buf[7] = u8Temp1;
            u8Temp0 = au8Buf[2];
            u8Temp1 = au8Buf[3];
            au8Buf[2] = au8Buf[4];
            au8Buf[3] = au8Buf[5];
            au8Buf[4] = u8Temp0;
            au8Buf[5] = u8Temp1;
        }
    }

    // Convert to little endian
    for (int i = 0; i < u8DataSize / 2; i++)
    {
        uint8_t u8Temp = au8Buf[i];
        au8Buf[i] = au8Buf[u8DataSize - 1 - i];
        au8Buf[u8DataSize - 1 - i] = u8Temp;
    }

    double dValue = 0;

    switch (u8DataType)
    {
    case MODBUS_INT16:
    {
        int16_t s16;
        memcpy(&s16, au8Buf, 2);
        dValue = s16;
        break;
    }

    case MODBUS_UINT16:
    {
        uint16_t u16;
        memcpy(&u16, au8Buf, 2);
        dValue = u16;
        break;
    }

    case MODBUS_INT32:
    {
        int32_t s32;
        memcpy(&s32, au8Buf, 4);
        dValue = s32;
        break;
    }

    case MODBUS_UINT32:
    {
        uint32_t u32;
        memcpy(&u32, au8Buf, 4);
        dValue = u32;
        break;
    }

    case MODBUS_FLOAT:
    {
        float f;
        memcpy(&f, au8Buf, 4);
        dValue = f;
        break;
    }

    case MODBUS_INT64:
    {
        int64_t s64;
        memcpy(&s64, au8Buf, 8);
        dValue = s64;
        break;
    }

    case MODBUS_UINT64:
    {
        uint64_t u64;
        memcpy(&u64, au8Buf, 8);
        dValue = u64;
        break;
    }

    case MODBUS_DOUBLE:
    {
        double d;
        memcpy(&d, au8Buf, 8);
        dValue = d;
        break;
    }

    default:
        return false;
    }

    // Apply multiplier (10^n)
    if (s8Multiplier < 0)
    {
        for (int i = 0; i < -s8Multiplier; i++)
        {
            dValue /= 10.0;
        }
    }
    else if (s8Multiplier > 0)
    {
        for (int i = 0; i < s8Multiplier; i++)
        {
            dValue *= 10.0;
        }
    }

    *pDataOut = dValue;
    return true;
}