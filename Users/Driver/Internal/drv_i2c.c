#include "drv_i2c.h"
#include "i2c.h"

void drv_i2c_init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x00201D2B;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
    {
        Error_Handler();
    }
}

void drv_i2c_deinit(void)
{
    HAL_I2C_DeInit(&hi2c1);
}

bool drv_i2c_send(uint16_t u16DevAddr, uint8_t *pData, uint16_t u16Size)
{
    return HAL_I2C_Master_Transmit(&hi2c1, u16DevAddr, pData, u16Size, I2C_POLLING_TIMEOUT) == HAL_OK;
}

bool drv_i2c_receive(uint16_t u16DevAddr, uint8_t *pData, uint16_t u16Size)
{
    return HAL_I2C_Master_Receive(&hi2c1, u16DevAddr, pData, u16Size, I2C_POLLING_TIMEOUT) == HAL_OK;
}