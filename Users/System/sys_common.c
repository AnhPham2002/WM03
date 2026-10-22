#include "sys_common.h"
#include "main.h"
#include "rng.h"

/*==================================================================================================
*                                PRIVATE FUNCTIONS DECLARATIONS
==================================================================================================*/

/**
 * @brief Configure the system clock.
 *
 * Initializes the MSI, LSI, and LSE oscillators and selects MSI as the system
 * clock source. Configures the AHB and APB bus prescalers and enables MSI
 * PLL mode.
 */
static void sys_clock_config(void);

/*==================================================================================================
*                                   PUBLIC FUNCTIONS DEFINITIONS
==================================================================================================*/

uint16_t sys_crc16(const uint8_t *pData, uint16_t u16Size)
{
    uint16_t u16Crc = 0xFFFF;
    for (uint16_t u16Pos = 0; u16Pos < u16Size; u16Pos++)
    {
        u16Crc ^= (unsigned int)pData[u16Pos]; // XOR byte into least sig. byte of crc

        for (int i = 8; i != 0; i--) // Loop over each bit
        {
            if ((u16Crc & 0x0001) != 0) // If the LSB is set
            {
                u16Crc = (u16Crc >> 1) ^ 0xA001; // Shift right and XOR 0xA001
            }
            else // Else LSB is not set
            {
                u16Crc >>= 1; // Just shift right
            }
        }
    }

    return u16Crc;
}

void sys_sleep(void)
{
    HAL_SuspendTick();
    HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
    sys_clock_config();
    HAL_ResumeTick();
}

uint32_t sys_get_random_number(void)
{
    bool bStatus = false;
    uint32_t u32RandomNumber;

    __HAL_RCC_RNG_CLK_ENABLE();
    if (HAL_RNG_Init(&hrng) == HAL_OK)
    {
        if (HAL_RNG_GenerateRandomNumber(&hrng, &u32RandomNumber) == HAL_OK)
        {
            bStatus = true;
        }
    }

    if (!bStatus)
    {
        // u32RandomNumber = ((uint32_t)rand() << 16) | (uint32_t)rand();
        u32RandomNumber = 0;
    }

    HAL_RNG_DeInit(&hrng);
    __HAL_RCC_RNG_CLK_DISABLE();

    return u32RandomNumber;
}

/*==================================================================================================
*                                   PRIVATE FUNCTIONS DEFINITIONS
==================================================================================================*/

static void sys_clock_config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = 0;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_7;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_RCCEx_EnableMSIPLLMode();
}