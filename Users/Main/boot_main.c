#include "boot_main.h"
#include "flash_layout.h"
#include "gpio.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_gpio.h"
#include <stdint.h>

#define VECTOR_MSP_OFFSET 0U
#define VECTOR_RESET_OFFSET 1U

typedef void (*pFunction)(void);

__attribute__((section(".boot_version"))) const uint8_t au8Verison[10] = "V1.0.1"; // Max version size: 10 bytes "Vxx.xx.xxx"

static uint32_t app_select(void);
static void jump_to_app(uintptr_t pAppAddr);
static void boot_blink_led(void);

void boot_main(void)
{
    jump_to_app(app_select());
    boot_blink_led();
}

static uint32_t app_select(void)
{
    return SLOT_A_START_ADDR;
}

static void jump_to_app(uintptr_t pAppAddr)
{
    const volatile uint32_t *vectorTable = (const volatile uint32_t *)pAppAddr;
    uint32_t appStack = vectorTable[VECTOR_MSP_OFFSET];
    uint32_t appResetHandler = vectorTable[VECTOR_RESET_OFFSET];
    pFunction resetHandler = (pFunction)appResetHandler;

    HAL_RCC_DeInit();
    HAL_DeInit();

    /* Disable all interrupt and turn off SystemTick */
    __disable_irq();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    /* Clear pending Interrupt Request */
    SCB->SHCSR &= ~(SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_MEMFAULTENA_Msk);

    /* Set main stack pointer */
    __set_MSP(appStack);

    __enable_irq();

    SCB->VTOR = pAppAddr;

    resetHandler();
}

static void boot_blink_led(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);

    /*Configure GPIO pin : PA15 */
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    while (1)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_15);
        HAL_Delay(100);
    }
}