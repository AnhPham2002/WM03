#include "drv_gpio.h"

static volatile bool bWkupPinFlag = false;

void drv_gpio_write(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, PinState);
}

bool drv_gpio_read(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    return HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
}

void drv_gpio_toggle(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    HAL_GPIO_TogglePin(GPIOx, GPIO_Pin);
}

void drv_gpio_deinit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    HAL_GPIO_DeInit(GPIOx, GPIO_Pin);
}

bool drv_gpio_irq_pending(void)
{
    if (bWkupPinFlag)
    {
        bWkupPinFlag = false;
        return true;
    }

    return false;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    bWkupPinFlag = true;
}