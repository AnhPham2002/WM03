#include "drv_spi.h"
#include "spi.h"

void drv_spi_init(void)
{
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 7;
    hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        Error_Handler();
    }
}

void drv_spi_deinit(void)
{
    HAL_SPI_DeInit(&hspi1);
}

bool drv_spi_send(const uint8_t *pData, uint16_t u16Size)
{
    return HAL_SPI_Transmit(&hspi1, pData, u16Size, SPI_POLLING_TIMEOUT) == HAL_OK;
}

bool drv_spi_receive(uint8_t *pData, uint16_t u16Size)
{
    return HAL_SPI_Receive(&hspi1, pData, u16Size, SPI_POLLING_TIMEOUT) == HAL_OK;
}