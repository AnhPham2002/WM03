#include "drv_modbus.h"

static uint8_t au8ModbusBuf[MODBUS_BUFFER_SIZE];
static uint16_t u16ModbusBufSize;

/*==================================================================================================
*                                PRIVATE FUNCTIONS DECLARATIONS
==================================================================================================*/

/**
 * @brief Calculate Modbus CRC16.
 *
 * @param[in] pData   Data buffer.
 * @param[in] u16Size Data size.
 *
 * @return CRC16 value.
 */
static uint16_t drv_modbus_crc16(uint8_t *pData, uint16_t u16Size);

/**
 * @brief Send Modbus frame.
 *
 * @param[in] u8SlaveAddr    Slave address.
 * @param[in] u8FunctionCode Function code.
 * @param[in] pData          Data buffer.
 * @param[in] u16DataSize    Data size.
 *
 * @return Modbus status.
 */
static Modbus_Status_t drv_modbus_send(uint8_t u8SlaveAddr, uint8_t u8FunctionCode, uint8_t *pData, uint16_t u16DataSize);

/**
 * @brief Receive and validate Modbus frame.
 *
 * @param[out] u8SlaveAddr    Slave address.
 * @param[out] u8FunctionCode Function code.
 * @param[out] pData          Data buffer.
 * @param[out] u16DataSize    Data size.
 *
 * @return Modbus status.
 */
static Modbus_Status_t drv_modbus_receive(uint8_t *u8SlaveAddr, uint8_t *u8FunctionCode, uint8_t *pData, uint16_t *u16DataSize);

/*==================================================================================================
*                                   PUBLIC FUNCTIONS DEFINITIONS
==================================================================================================*/

void drv_modbus_init(uint32_t u32Baud, uint32_t u32Parity)
{
    drv_uart1_init(u32Baud, u32Parity);
}

Modbus_Status_t drv_modbus_master_read_register(uint8_t u8SlaveAddr, uint8_t u8FuncCode, uint16_t u16StartRegAddr, uint16_t u16RegQuantity, uint8_t *pData, uint16_t *u16DataSize)
{
    if ((u8FuncCode != MODBUS_FUNC_READ_HOLDING_REGISTERS) && (u8FuncCode != MODBUS_FUNC_READ_INPUT_REGISTERS))
    {
        return MODBUS_ERROR;
    }

    drv_rs485_pwr_on();

    uint8_t au8TxData[] = {(uint8_t)(u16StartRegAddr >> 8), (uint8_t)(u16StartRegAddr & 0xFF), (uint8_t)(u16RegQuantity >> 8), (uint8_t)(u16RegQuantity & 0xFF)};
    if (drv_modbus_send(u8SlaveAddr, u8FuncCode, au8TxData, sizeof(au8TxData)) != MODBUS_OK)
    {
        return MODBUS_ERROR;
    }

    Modbus_Status_t eModbusStatus;
    uint8_t u8RxSlaveAddr;
    uint8_t u8RxFuncCode;
    uint8_t au8RxData[20];
    uint16_t u16RxDataSize;
    uint32_t u32TimeStart = sys_time_ms();
    do
    {
        eModbusStatus = drv_modbus_receive(&u8RxSlaveAddr, &u8RxFuncCode, au8RxData, &u16RxDataSize);
        if (sys_time_ms() - u32TimeStart >= MODBUS_RESPONSE_TIMEOUT)
        {
            break;
        }
    }
    while (eModbusStatus != MODBUS_OK);

    drv_rs485_pwr_off();

    if (eModbusStatus != MODBUS_OK)
    {
        return eModbusStatus;
    }

    if (u8RxFuncCode == u8FuncCode + 0x80)
    {
        *u16DataSize = 1; // Exception code size = 1 byte
        memcpy(pData, &au8RxData[0], *u16DataSize); // Copy exception code

        return MODBUS_EXCEPTION;
    }

    *u16DataSize = u16RegQuantity * 2;
    memcpy(pData, &au8RxData[1], *u16DataSize); // Byte 0 is byte count

    return eModbusStatus;
}

/*==================================================================================================
*                                   PRIVATE FUNCTIONS DEFINITIONS
==================================================================================================*/

static uint16_t drv_modbus_crc16(uint8_t *pData, uint16_t u16Size)
{
    return sys_crc16(pData, u16Size);
}

static Modbus_Status_t drv_modbus_send(uint8_t u8SlaveAddr, uint8_t u8FunctionCode, uint8_t *pData, uint16_t u16DataSize)
{
    if ((pData == NULL) || (u16DataSize == 0))
    {
        return MODBUS_ERROR;
    }

    uint16_t u16FrameLen = 1 + 1 + u16DataSize + 2; // 1 byte Slave Address, 1 byte Function Code, 2 bytes CRC

    au8ModbusBuf[0] = u8SlaveAddr;
    au8ModbusBuf[1] = u8FunctionCode;
    memcpy(&au8ModbusBuf[2], pData, u16DataSize);

    uint16_t u16Crc = drv_modbus_crc16(au8ModbusBuf, u16FrameLen - 2);
    au8ModbusBuf[u16FrameLen - 2] = (uint8_t)(u16Crc & 0xFF);
    au8ModbusBuf[u16FrameLen - 1] = (uint8_t)(u16Crc >> 8);

    if (drv_rs485_send(au8ModbusBuf, u16FrameLen))
    {
        return MODBUS_OK;
    }

    return MODBUS_ERROR;
}

static Modbus_Status_t drv_modbus_receive(uint8_t *u8SlaveAddr, uint8_t *u8FunctionCode, uint8_t *pData, uint16_t *u16DataSize)
{
    if ((u8SlaveAddr == NULL) || (u8FunctionCode == NULL) || (pData == NULL) || (u16DataSize == NULL))
    {
        return MODBUS_ERROR;
    }

    if (drv_rs485_receive(au8ModbusBuf, &u16ModbusBufSize))
    {
        if (u16ModbusBufSize < 4U)
        {
            return MODBUS_ERROR;
        }

        uint16_t u16Crc = (uint16_t)(au8ModbusBuf[u16ModbusBufSize - 1] << 8) | (uint16_t)(au8ModbusBuf[u16ModbusBufSize - 2]);
        if (u16Crc != drv_modbus_crc16(au8ModbusBuf, u16ModbusBufSize - 2))
        {
            return MODBUS_CRC_ERROR;
        }

        *u8SlaveAddr = au8ModbusBuf[0];
        *u8FunctionCode = au8ModbusBuf[1];
        *u16DataSize = u16ModbusBufSize - 1 - 1 - 2; // 1 byte Slave Address, 1 byte Function Code, 2 bytes CRC
        memcpy(pData, &au8ModbusBuf[2], *u16DataSize);
        return MODBUS_OK;
    }

    return MODBUS_NO_DATA;
}